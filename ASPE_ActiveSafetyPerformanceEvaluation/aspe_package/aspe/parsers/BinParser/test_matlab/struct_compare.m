function struct_compare(path_to_pars_out, path_to_bin)
    bin = '*.bin';
    mat = '*.mat';
    bin_files = dir(fullfile(path_to_bin, bin));
    mat_files = dir(fullfile(path_to_pars_out, mat));
    
    for i=1:length(bin_files)
       bin_file_name{i} = erase(bin_files(i).name,'.bin'); 
    end
    for i=1:length(mat_files)
       mat_file_name{i} = erase(mat_files(i).name,'.mat'); 
    end   
    
    for idx=1:length(mat_file_name)
        bin_match = find(contains(bin_file_name,mat_file_name{idx}));
        if ~isempty(bin_match)
            bin_data = read_log_data(fullfile(path_to_bin, bin_files(bin_match).name));
            mat_data = load(fullfile(path_to_pars_out, mat_files(idx).name));
            if isequalwithequalnans(bin_data, mat_data)
                disp([mat_file_name{idx},'.mat matched to ', bin_files(bin_match).name]);
            else
                disp(['mismatch detected between',mat_file_name{idx},'.mat and', bin_files(bin_match).name]);
            end
        else
            disp(['no matching bin file for ',mat_file_name{idx},'.mat']);
        end
    end

   
    
    