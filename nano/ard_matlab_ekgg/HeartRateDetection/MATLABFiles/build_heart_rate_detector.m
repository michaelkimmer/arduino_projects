%%

% Copyright 2018 The MathWorks, Inc.
T = qrs_types('fixed');
replacement_lib = 'hand_written'; %'hong' % 'dst', % 'hand_written'

x = cast(0, 'like', T.arduino_input);
cfg = coder.config('lib'); %#ok<*UNRCH>
if isfi(T.data) && isfixed(T.data)
    cfg.PurelyIntegerCode = true;
end
cfg.SupportNonFinite = false;
cfg.MATLABFcnDesc = true;
%cfg.MATLABSourceComments = true;
cfg.GenCodeOnly = true;
cfg.TargetLang = 'C';
cfg.FilePartitionMethod = 'SingleFile';
cfg.HardwareImplementation.ProdHWDeviceType = 'Atmel->AVR';
cfg.SaturateOnIntegerOverflow = false;
codegen heart_rate_detector_arduino_interface -args {x, T, coder.Constant(replacement_lib)} -config cfg
disp('Done compiling')
%%
% cpp_files = fullfile(pwd,'codegen','lib','heart_rate_detector_arduino_interface','*.cpp');
% header_files = fullfile(pwd,'codegen','lib','heart_rate_detector_arduino_interface','*.h');
arduino_source = fullfile(pwd,'..','..','heart_monitor');

% copyfile(cpp_files,arduino_source,'f');
% copyfile(header_files,arduino_source,'f');
%%
copy_files_to_arduino_project('heart_rate_detector_arduino_interface', arduino_source)