function copy_files_to_arduino_project(function_name, arduino_source)
% Copyright 2018 The MathWorks, Inc.    
    % Change .c to .cpp for Arduino
    codegen_dir = fullfile(pwd,'codegen','lib',function_name);
    c_files = fullfile(codegen_dir,'*.c');
    d = dir(c_files);
    for i = 1:length(d)
        movefile(fullfile(codegen_dir,d(i).name),fullfile(codegen_dir,[d(i).name,'pp']),'f');
    end
    cpp_files = fullfile(pwd,'codegen','lib',function_name,'*.cpp');
    header_files = fullfile(pwd,'codegen','lib',function_name,'*.h');
    %arduino_source = fullfile(pwd,arduino_project_name);
    %arduino_source_cpp = fullfile(pwd,arduino_project_name);
    
    copyfile(cpp_files,arduino_source,'f');
    copyfile(header_files,arduino_source,'f');
    
end