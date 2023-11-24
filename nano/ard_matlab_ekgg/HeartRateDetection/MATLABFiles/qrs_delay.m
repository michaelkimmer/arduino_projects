function [QRSDelay,thisThreshold] = qrs_delay(pk, init)
    % Copyright 2018 The MathWorks, Inc.
    persistent count
    persistent lastQRSDelay
    persistent threshold
    persistent lastPeak
    if isempty(lastQRSDelay) || init
        count = 0;
        lastQRSDelay = 0;
        threshold = 4;
        lastPeak = 0;
    end
    count = count + 1;
    threshold = filter8(pk);
    if pk > threshold
        lastQRSDelay = count;
        count = 0;
    end
    QRSDelay = lastQRSDelay;
    thisThreshold = threshold;
end

function y = filter8(x)
    persistent n
    persistent z
    persistent lastY
    if isempty(n)
        n = 0;
        z = zeros(8,1);
        lastY = 0;
    end
    if x ~= 0
        z(n+1) = x;
        n = mod(n+1,8);
        lastY = sum(z)/2^3;
    end
    y = lastY;
end