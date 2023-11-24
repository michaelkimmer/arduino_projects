close all force; clear; clc;



hfig = uifigure();
hfig.UserData.Arduino_Position = [0,0,0];


hax1 = uiaxes(hfig, 'Position', [hfig.Position(3)-hfig.Position(4),1, hfig.Position(4) ,hfig.Position(4)]);
    hax1.XLim = [-1, 1];
    hax1.YLim = [-1, 1];
    hax1.ZLim = [-1, 1];
grid(hax1, 'on');

hbutt1 = uibutton(hfig, 'Position', [hax1.Position(1)*1/8, 25, hax1.Position(1)*6/8, 20], 'Text', 'Set axes radius');

h_editfield = uieditfield(hfig, 'Position', [hax1.Position(1)*1/8, 50, hax1.Position(1)*6/8, 20]);

Arduino_Point = scatter3(hax1, hfig.UserData.Arduino_Position(1), hfig.UserData.Arduino_Position(2), hfig.UserData.Arduino_Position(3), 'filled');

%% set callback
hbutt1.ButtonPushedFcn = @(btn,event)Button1Pushed(btn,h_editfield, hax1);



%% connect arduino
serialportlist("available")' 
arduinoObj = serialport("COM13",9600) %!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
%configure Terminator
configureTerminator(arduinoObj,"CR/LF");
%Flush the serialport object to remove any old data.
flush(arduinoObj);
% the UserData property to store the Arduino data
arduinoObj.UserData = struct("Data",[],"Count",0)

%set callback (Set the BytesAvailableFcnMode property to "terminator")
configureCallback(arduinoObj,"terminator",@readSineWaveData);






%% serial port callbaack
function readSineWaveData(src, ~)

% Read the ASCII data from the serialport object.
data = readline(src);

% Convert the string data to numeric type and save it in the UserData
% property of the serialport object.
src.UserData.Data(end+1) = str2double(data);

% Update the Count value of the serialport object.
src.UserData.Count = src.UserData.Count + 1;

% If 3 data points have been collected from the Arduino, plot the data.
if(src.UserData.Count >= 3)
    Arduino_Point.XData = src.UserData.Data(1);
    Arduino_Point.YData = src.UserData.Data(2);
    Arduino_Point.ZData = src.UserData.Data(3);

    %remove data
    src.UserData.Data = [];
    src.UserData.Count = 0;
end

end

%% callback function button 
function Button1Pushed(btn,h_editfield, hax1)
        Value = str2num(h_editfield.Value);
        if(~isempty(Value))
            hax1.XLim = [-Value, Value];
            hax1.YLim = [-Value, Value];
            hax1.ZLim = [-Value, Value];
        else
            warning("Entered value is not numeric");
        end
end













