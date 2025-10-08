import os

import numpy as np
import pandas as pd
from PIL import Image, ImageDraw, ImageFont
from PyQt5.Qt import QFileDialog

from aspe.utilities.SupportingFunctions import get_dec_rgb


class GifCreator:

    cfg = {
        'rgb2bgr':                      [2, 1, 0],
        'character_width_factor':       1.5,
        'legend_gap_space':             50,
        'text_spacing':                 5,
        'init_title_position':          [25, 0],
        'rgb_white':                    (255, 255, 255),
    }

    def __init__(self, scan_exporter):
        self._scan_exporter = scan_exporter
        self.drawers_controller = None
        self._legend_df = pd.DataFrame()
        self._frames = []
        self._scan_index = []
        self._title_font_size = 17
        self._legend_font_size = 17
        self._font_type = ""
        self._title_log_name = ""

    def save_current_scan_index(self, scan_index, size):
        # Set images size as bird eye view dimensions
        self._scan_exporter.parameters()["width"] = size.width()
        self._scan_exporter.parameters()["height"] = size.height()
        scan_qimage = self._scan_exporter.export(toBytes=True)
        self._frames.append(scan_qimage.data[:, :, self.cfg['rgb2bgr']])
        self._scan_index.append(scan_index)

    def create_gif(self, last_dir):
        self._legend_df, self._title_log_name = self.drawers_controller.get_legend()
        # Get path to the system fonts
        system_drive = os.getenv("SYSTEMDRIVE")
        self._font_type = system_drive + r"\Windows\Fonts\Arial.ttf"
        # Prepare legend parameters
        width = self._scan_exporter.parameters()["width"]
        if self._legend_df.shape[0]:
            # Calculate legend font size
            row_pixels_to_use = width - self.cfg['legend_gap_space']
            longest_log_name = max(self._legend_df["characters_per_legend_line"])
            character_width = int(self.cfg['character_width_factor'] * row_pixels_to_use / longest_log_name)
            self._legend_font_size = min(self._legend_font_size, character_width)
            legend_rows = self._legend_df.shape[0]
            # Calculate title font size
            if self._title_log_name:  # check if string is not empty
                name_length = len(self._title_log_name)
                character_width = int(self.cfg['character_width_factor'] * row_pixels_to_use / name_length)
                self._title_font_size = min(self._title_font_size, character_width)
        else:
            legend_rows = 0

        # Increase scan image size for legend and title placing
        legend_additional_space = \
            np.zeros(((legend_rows + 1) * (self._legend_font_size + self.cfg['text_spacing']), width, 3),
                     dtype=np.uint8)
        title_additional_space = np.zeros((2 * (self._title_font_size + self.cfg['text_spacing']), width, 3),
                                          dtype=np.uint8)
        self._frames = [np.vstack([title_additional_space, frame, legend_additional_space]) for frame in self._frames]
        self._add_title()
        if self._legend_df.shape[0]:
            self._add_legend()
        save_path, _ = QFileDialog.getSaveFileName(directory=last_dir, filter="Gif files (*.gif)")
        if save_path:
            im = Image.fromarray(self._frames[0])
            images = [Image.fromarray(img) for img in self._frames]
            im.save(save_path, save_all=True, append_images=images[1:], optimize=True, loop=0)
        self._frames = []
        self._scan_index = []

    def _add_title(self):
        # Displaying plot title and current scan index
        title_font = ImageFont.truetype(self._font_type, self._title_font_size)
        for i in range(len(self._frames)):
            # add plot title
            title_position = self.cfg['init_title_position'].copy()
            img = Image.fromarray(self._frames[i])
            draw = ImageDraw.Draw(img)
            text = self._title_log_name
            title_position[0] = \
                int((self._scan_exporter.parameters()["width"] - len(text) * (self._title_font_size / 1.5)) / 2)
            draw.text((title_position[0], title_position[1]), text, self.cfg['rgb_white'], font=title_font)
            # add current scan index under plot title
            text = "scan index: " + str(self._scan_index[i])
            title_position[1] += self._title_font_size + self.cfg['text_spacing']
            draw.text((title_position[0], title_position[1]), text, self.cfg['rgb_white'], font=title_font)
            self._frames[i] = np.array(img)

    def _add_legend(self):
        # Displaying plot legend
        # 25 is distance from the left end of image
        position_x_legend = self.cfg['legend_gap_space'] // 2
        # 5 is pixels amount to create a gap between rows
        position_y_increase_value = self._legend_font_size + self.cfg['text_spacing']
        # increase at the start due to legend title
        position_y_legend = self._scan_exporter.parameters()["height"] + (self._title_font_size
                                                                          + self.cfg['text_spacing']) * 2
        title_font = ImageFont.truetype(self._font_type, self._legend_font_size)
        for i in range(len(self._frames)):
            img = Image.fromarray(self._frames[i])
            draw = ImageDraw.Draw(img)
            legend_position = [position_x_legend, position_y_legend]
            for idx, _ in self._legend_df.iterrows():
                # add colored object symbol
                text = self._legend_df.loc[idx, "style"]
                r, g, b = get_dec_rgb(self._legend_df.loc[idx, "color"])
                draw.text((legend_position[0], legend_position[1]), text, (r, g, b), font=title_font)
                # add object description
                legend_position[0] += int(self._legend_font_size) * 2
                text = self._legend_df.loc[idx, "log_nick_name"] + " - " + self._legend_df.loc[idx, "drawer_name"] \
                    + " - " + self._legend_df.loc[idx, "subdrawer_name"]
                draw.text((legend_position[0], legend_position[1]), text, self.cfg['rgb_white'], font=title_font)
                # change legend position for next row
                legend_position[0] = position_x_legend
                legend_position[1] += position_y_increase_value
                self._frames[i] = np.array(img)
