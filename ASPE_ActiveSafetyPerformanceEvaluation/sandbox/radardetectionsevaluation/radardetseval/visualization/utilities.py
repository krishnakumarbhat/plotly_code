import matplotlib.axes


def reduce_labels(legend_handlers, labels):
    """
    Reduce duplicated entries in legend

    :param legend_handlers:
    :param labels:
    :return:
    """
    unique_legend_handlers = []
    unique_labels = []
    for legend_handler, label in zip(legend_handlers, labels):
        if label not in unique_labels:
            unique_labels.append(label)
            unique_legend_handlers.append(legend_handler)
    return unique_legend_handlers, unique_labels


def publish_reduced_labels(axes: matplotlib.axes.Axes):
    """
    Plot labels with handling multiplication of labels - something is visualized several times
    :param axes: axes handler for which legend should be published.
    :return:
    """
    legend_handlers, labels, = axes.get_legend_handles_labels()
    unique_legend_handlers, unique_labels, = reduce_labels(legend_handlers, labels)
    axes.legend(unique_legend_handlers, unique_labels)
