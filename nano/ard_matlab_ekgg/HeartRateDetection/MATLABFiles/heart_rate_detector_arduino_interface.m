function heart_rate = heart_rate_detector_arduino_interface(x,T,replacement_lib)
% Maintain the int16 output for the Arduino interface.  
% Expects input x to be signed 16-bit fixed-point or integer.

% Copyright 2018 The MathWorks, Inc.
    assert((isfi(x) && isfixed(x) && x.WordLength == 16 && x.SignednessBool) || ...
           isa(x,'int16'));
    hr = heart_rate_detector(x, T, replacement_lib);
    heart_rate = cast(hr, 'like', T.heart_rate_output);
end
