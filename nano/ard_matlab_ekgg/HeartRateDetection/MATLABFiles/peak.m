function pk = peak(datum, init)
    % peak() takes a datum as input and returns a peak height
    % when the signal returns to half its peak height, or it has been
    % 95 ms since the peak height was detected.
    % Copyright 2018 The MathWorks, Inc.
    persistent maxPeak lastDatum timeSinceMax
    pk = 0;
    if isempty(maxPeak) || init
        maxPeak = 0;
        lastDatum = 0;
        timeSinceMax = 0;
        return
    end
    
    if timeSinceMax > 0
        timeSinceMax = timeSinceMax + 1;
    end
    
    if((datum > lastDatum) && (datum > maxPeak))
        maxPeak = datum ;
        if(maxPeak > 2)
            % Not sure what maxPeak > 2 means with scaling?
            timeSinceMax = 1 ;
        end
    elseif(datum < maxPeak/2)
        % Less than half the peak height
        pk = maxPeak ;
        maxPeak = 0 ;
        timeSinceMax = 0 ;
    elseif(timeSinceMax > MS95)
        pk = maxPeak ;
        maxPeak = 0 ;
        timeSinceMax = 0 ;
    end
    lastDatum = datum ;
end