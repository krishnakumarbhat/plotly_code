import numpy as np


def calc_occupancy_grid(
        x_tick,
        y_tick,
        x_offset,
        y_offset,
        x_n_cells_close,
        x_n_cells_mid,
        x_n_cells_far,
        y_n_cells,
        factor,
        angle,
        curvature,
        inverse_cs_flag=True):
    """
    Base function to calculate occupancy grid coordinates. Body can be edited to match OCG shape.

    @param x_tick: Tick between following x cells points.
    @param y_tick: Tick between following y cells points.
    @param x_offset: OCG grid x offset in VCS.
    @param y_offset: OCG grid y offset in VCS.
    @param x_n_cells_far: Number of cells in x-axis, furthest from host.
    @param x_n_cells_mid: Number of cells in x-axis, in mid-section of the grid.
    @param x_n_cells_close: Number of cells in x-axis, closest to the host.
    @param y_n_cells: Number of cells along y-axis.
    @param factor: Grid scaling factor.
    @param angle:  Grid rotation angle.
    @param curvature: Host curvature.
    @param inverse_cs_flag: Flag to inverse x and y axes.
    @return: x and y paths for current scan index
    """
    if inverse_cs_flag:
        x_tick, y_tick, x_offset, y_offset, x_n_cells, y_n_cells_close, y_n_cells_mid, y_n_cells_far = inverse_cs(
            x_tick,
            y_tick,
            x_offset,
            y_offset,
            x_n_cells_close,
            x_n_cells_mid,
            x_n_cells_far,
            y_n_cells)
    cs_origin = np.array([0, 0])
    x = np.array([cs_origin[0], x_tick, x_tick, cs_origin[0], cs_origin[0], np.nan])
    y = np.array([cs_origin[1], cs_origin[1], y_tick, y_tick, cs_origin[1], np.nan])

    y_n_cells = y_n_cells_close + y_n_cells_mid + y_n_cells_far
    n_cells_grid = y_n_cells * x_n_cells

    x_result, y_result = create_polygon_path_coordinates(x, y, x_n_cells, y_n_cells, x_tick, y_tick)
    x_result = change_cs_origin(x_result, x_tick, x_n_cells)
    x_result, y_result = scale_grid_by_factor(x_result,
                                              y_result,
                                              factor,
                                              y_tick,
                                              y_n_cells_close,
                                              y_n_cells_mid,
                                              cs_origin,
                                              n_cells_grid)
    x_result = curvilinear_transformation(x_result, y_result, curvature, n_cells_grid)
    x_result, y_result = rotate(x_result, y_result, angle)
    x_result, y_result, cs_origin = translate_coordinates_by_offset(x_result, y_result, x_offset, y_offset, cs_origin)

    if inverse_cs_flag:
        x_result, y_result = convert_back_to_vcs(x_result, y_result)

    return x_result, y_result


def create_polygon_path_coordinates(x, y, x_n_cells, y_n_cells, x_tick, y_tick):
    n_polygon_points = 6
    # create mask of points, to translate current x coordinates
    x_mask = np.repeat(np.arange(x_n_cells), n_polygon_points) * x_tick
    # tile x coordinates to match number of x cells
    x_tiled = np.tile(x, x_n_cells)
    # add mask to tile x coordinates
    x_result = x_tiled + x_mask
    # tile x coordinates to match number of y cells
    x_result = np.tile(x_result, y_n_cells)

    # create mask of points, to translate current y coordinates
    y_mask = np.repeat(np.arange(y_n_cells), n_polygon_points) * y_tick
    # expand the mask to match number of grid cells
    y_mask = np.repeat(y_mask, x_n_cells)
    # tile y coordinates to match number of grid cells
    y_tiled = np.tile(y, x_n_cells * y_n_cells)
    # add mask to y coordinates
    y_result = y_tiled + y_mask

    return x_result, y_result


def translate_coordinates_by_offset(x_result, y_result, x_off, y_off, cs_origin):
    x_result = x_result + x_off
    y_result = y_result + y_off
    cs_origin[0] = cs_origin[0] + x_off
    cs_origin[1] = cs_origin[1] + y_off

    return x_result, y_result, cs_origin


def change_cs_origin(x_result, x_tick, x_n_cells):
    offset = x_n_cells * x_tick / 2
    x_result = x_result - offset

    return x_result


def scale_grid_by_factor(x, y, factor, y_tick, y_n_close, y_n_mid, cs_origin, n_cells_grid):
    def create_scale_factor_vector(factor, n_cells):
        diff = factor - 1
        factor_vector = np.zeros(shape=n_cells + 1)
        factor_step = diff / n_cells
        if diff != 0:
            to_append = 1
            for i in range(0, n_cells + 1):
                factor_vector[i] = to_append
                to_append += factor_step
        return factor_vector

    # don't scale if factor is equal 1
    if factor == 1:
        return x, y

    factor_vector = create_scale_factor_vector(factor, y_n_mid)

    if factor_vector.size == 0:
        return x, y

    y_start = y_tick * y_n_close + cs_origin[1]  # calculate y where to start scaling
    y_stop = y_start + y_tick * y_n_mid  # calculate y where to stop scaling

    mask_scale = np.logical_and(y >= y_start,  y <= y_stop)

    x_scale = x[mask_scale]
    y_scale = y[mask_scale]

    # calculate differences vector
    diff_y = y_scale - cs_origin[1]
    # indexes to sort array
    idx = np.argsort(diff_y)
    # indexes to undo sorting
    ids = np.argsort(idx)
    # sort x by differences
    x_sorted = x_scale[idx]
    # find unique values and their counts in differences
    values, counts = np.unique(diff_y, return_counts=True)
    # expand factor vector by differences counts to match sorted x
    factor_vector_repeated = np.repeat(factor_vector, counts)
    # multiply sorted x and expanded factor vector; indexing used to drop nans
    x_sorted[:n_cells_grid * 5] *= factor_vector_repeated
    # undo x sorting
    x[mask_scale] = x_sorted[ids]

    # scale by max factor after saturation
    mask_saturation = y > y_stop
    x[mask_saturation] = x[mask_saturation] * factor
    return x, y


def curvilinear_transformation(x, y, curvature, n_cells_grid):
    if curvature != 0:
        radius = 1 / curvature

        # transformed circle equation, (x - x0)^2 + (y - y0)^2 = r^2, where x0 = r and y0 = 0
        # after transformation equation is given in form: x = sqrt(r^2 - y^2) + r, but only when r > y
        # when the radius is positive: x = -sqrt(r^2 - y^2) + r
        # when the radius is negative: x = sqrt(r^2 - y^2) + r

        y_points, counts = np.unique(y, return_counts=True)
        # drop nan
        y_points = y_points[:-1]

        if abs(radius) > np.max(y_points):
            if radius > 0:
                x_circle = -np.sqrt(radius ** 2 - y_points ** 2) + radius
            elif radius < 0:
                x_circle = np.sqrt(radius ** 2 - y_points ** 2) + radius

            # get indexes sorted by y value
            idx = np.argsort(y)
            # get indexes to undo sorting
            ids = np.argsort(idx)
            # sort x with sorted y indexes
            x_sorted = x[idx]
            # expand x_circle to match x size
            x_circle_repeated = np.repeat(x_circle, counts[:-1])
            # translate x coordinates, indexing used to drop nans
            x_sorted[:-n_cells_grid] += x_circle_repeated
            # undo sorting
            return x_sorted[ids]
    return x


def rotate(x, y, angle):
    paired = np.concatenate((x.reshape(1, -1), y.reshape(1, -1)), axis=0)
    size = paired.shape[1]
    pos_mat = paired.reshape(2, size, 1).transpose(1, 2, 0)

    rot_mat = np.tile(np.array([[np.cos(angle), -np.sin(angle)],
                                [np.sin(angle), np.cos(angle)]]).reshape(2, 2, 1).transpose(2, 1, 0), (size, 1, 1))

    result = pos_mat @ rot_mat

    split = np.vsplit(result.transpose(2, 0, 1).reshape(2, size), 2)

    x_result = split[0].reshape(-1)
    y_result = split[1].reshape(-1)

    return x_result, y_result


def inverse_cs(
        x_tick,
        y_tick,
        x_offset,
        y_offset,
        x_n_cells_close,
        x_n_cells_mid,
        x_n_cells_far,
        y_n_cells):

    x_tick, y_tick = y_tick, x_tick
    x_offset, y_offset = y_offset, x_offset
    x_n_cells, y_n_cells_close, y_n_cells_mid, y_n_cells_far = y_n_cells, x_n_cells_close, x_n_cells_mid, x_n_cells_far

    return x_tick, y_tick, x_offset, y_offset, x_n_cells, y_n_cells_close, y_n_cells_mid, y_n_cells_far


def convert_back_to_vcs(x_result, y_result):
    x_result, y_result = y_result, x_result
    return x_result, y_result
