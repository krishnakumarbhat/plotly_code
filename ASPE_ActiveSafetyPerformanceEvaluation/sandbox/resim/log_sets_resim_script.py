from sandbox.resim.F360ResimRunner import F360ResimRunner, TxtReader

dvl_log_lists = [
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\logs0.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\logs1.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\logs2.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\logs3.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\logs4.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\logs5.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\logs6.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\logs7.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\logs8.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\logs9.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\logs10.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\logs11.txt"]

ini_file_list = [
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\ini0.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\ini1.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\ini2.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\ini3.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\ini4.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\ini5.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\ini6.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\ini7.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\ini8.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\ini9.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\ini10.txt",
    r"\\10.224.186.68\AD-Shared\F360\Logs\DataSets\resim_test_for_hpcc\ini11.txt"]

resim_exe_path = r"\\10.224.186.68\AD-Shared\F360\Tools\Resim\Standalone\F360Core_2_08_0\resim_f360.exe"
runner = F360ResimRunner(resim_exe_path)

for dvl_list_txt, ini_file_txt in zip(dvl_log_lists, ini_file_list):
    dvl_files_list = TxtReader.get_multi_lines(dvl_list_txt)
    ini_file = TxtReader.get_single_line(ini_file_txt)
    runner.resim_log_list(dvl_files_list, ini_file)
