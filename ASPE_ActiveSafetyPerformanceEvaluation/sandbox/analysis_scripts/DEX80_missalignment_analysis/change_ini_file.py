import os


def create_new_ini_file_with_align_corr_angle(ini_path, angle_deg):
    ini_lines = read_ini_file_lines(ini_path)
    set_allign_from_can(ini_lines)
    add_missalignment_correction_angle(angle_deg, ini_lines)
    folder, name_ext = os.path.split(ini_path)
    name, _ = os.path.splitext(name_ext)
    angle_deg_str = str(angle_deg).replace('.', '_').replace('-', 'neg_')
    name_save_file = f'{name}_align_angle_{angle_deg_str}_case_2.ini'
    save_path = os.path.join(folder, name_save_file)
    save_ini_file(save_path, ini_lines)
    return save_path

def read_ini_file_lines(ini_path):
    with open(ini_path, 'r') as file:
        ini_lines = file.readlines()
    return ini_lines


def set_allign_from_can(ini_lines):
    for idx, line in enumerate(ini_lines):
        if 'Align_FromCAN' in line:
            ini_lines[idx] = 'Align_FromCAN=0\n'


def add_missalignment_correction_angle(cor_angle_deg, ini_lines):
    corr_angle_vec = [cor_angle_deg, cor_angle_deg, cor_angle_deg, cor_angle_deg]
    deg_idx = 0
    for idx, line in enumerate(ini_lines):
        if 'KA_MisAlignAngle_AZ_deg' in line:
            curr_ang = float(line.split('=')[1].split(';')[0])
            corrected_angle = curr_ang + corr_angle_vec[deg_idx]
            ini_lines[idx] = f'KA_MisAlignAngle_AZ_deg={corrected_angle}\n'
            deg_idx += 1


def save_ini_file(path, lines):
    with open(path, 'w') as file:
        file.writelines(lines)


if __name__ == "__main__":
    ini_path = r"C:\logs\OXTS_RT-Range\RNA_SRR5\Opole_CW19\F360Tracker_RNA_SUV_Espace.ini"
    create_new_ini_file_with_align_corr_angle(ini_path, 1.23)

