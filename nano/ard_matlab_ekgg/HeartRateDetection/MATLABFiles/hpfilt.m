function y = hpfilt(u)
    %  hpfilt() implements the high pass filter represented by the following
    %  difference equation:
    %
    %       x[n] = x[n-1] + u[n] - u[n-32]
    %       y[n] = u[n-16] - x[n] ;
    %
    %  Note that the filter delay is 15.5 samples
    
    % Circular buffer n
  % Copyright 2018 The MathWorks, Inc.  
    coder.inline('never');
    persistent x data n
    if isempty(x)
        x = zeros(1,1,'like',u);
        data = zeros(32,1,'like',u);
        n = int16(0);
    end
    x(:) = x + u - data(n+1);
    half_n = bitand(n - 16, int16(31));
    
    y = data(half_n+1);
    y(:) = y - bitsra(x,5);
    data(n+1) = u;
    n(:) = bitand(n + 1, int16(31));
end

