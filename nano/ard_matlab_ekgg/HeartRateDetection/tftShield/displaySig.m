classdef displaySig < matlab.System & matlab.system.mixin.Propagates ...
        & matlab.system.mixin.CustomIcon & coder.ExternalDependency
    % displaySig
    %
   
    %#codegen
    %#ok<*EMCA>
    
    % Public, non-tunable properties
    properties(Nontunable)
        %Line Color
        line1Color_ = 'Green';
        line2Color_ = 'Red';
    end

    properties(Constant, Hidden)
        line1Color_Set = matlab.system.StringSet({'Black','Navy','Darkgreen','Darkcyan','Maroon','Purple','Olive','Lightgrey','Darkgrey',...
                         'Green','Cyan','Red','Magenta','Yellow','White','Orange','Greenyellow','Pink'});
        line2Color_Set = matlab.system.StringSet({'Black','Navy','Darkgreen','Darkcyan','Maroon','Purple','Olive','Lightgrey','Darkgrey',...
                         'Green','Cyan','Red','Magenta','Yellow','White','Orange','Greenyellow','Pink'});    
        spacing = 0.25;
    end
    % Pre-computed constants
    properties(Access = private)
        ColorCode = {'Black','Navy','Darkgreen','Darkcyan','Maroon','Purple','Olive','Lightgrey','Darkgrey',...
                         'Green','Cyan','Red','Magenta','Yellow','White','Orange','Greenyellow','Pink'};
        counter=uint16(0); 
        x1=uint16(0);
        x2=uint16(0);
        y1=uint16(0);
        y2=uint16(0);
        y3=uint16(0);
        y4=uint16(0);
        Color1Index=uint8(0);
        Color2Index=uint8(0);
    end

    methods
        % Constructor
        function obj = displaySig(varargin)
            % Support name-value pair arguments when constructing object
            setProperties(obj,nargin,varargin{:})
        end
    end

    methods(Access = protected)
        %% Common functions
        function setupImpl(obj)
            % Perform one-time calculations, such as computing constants
            if coder.target('Rtw')
                [~,c] = size(obj.ColorCode);
                for i = 1:c
                    if(strcmp(obj.line1Color_,obj.ColorCode{i}))
                        obj.Color1Index = uint8(i-1);    
                    end
                    if(strcmp(obj.line2Color_,obj.ColorCode{i}))
                        obj.Color2Index = uint8(i-1);    
                    end
                end
                coder.cinclude('heart_monitor.h')
                coder.ceval('tftSetup');
            end
        end

        function stepImpl(obj,sensorValue, hp)
            tW = uint16(0);
            tH = uint16(0);
            if coder.target('Rtw')
                senVal = (sensorValue*8)+100;
                hpNew = (hp*8)+100;
                tW = coder.ceval('getTftWidth');
                if(obj.counter > (tW/obj.spacing))
                    obj.counter = uint16(0);
                    coder.ceval('setTftFillscreen');
                end
%                 senVal = senVal / 8 ; % Right shift by three bits
                tH = coder.ceval('getTftHeight');
                if(obj.counter == 0)
                    obj.x1 = uint16(0);
                    obj.x2 = uint16(0);
                    obj.y1 = tH - uint16(senVal);
                    obj.y3 = tH - uint16(hpNew);
                end
                if(obj.counter > 0)
                    obj.x2 = obj.counter * obj.spacing;
                    obj.y2 = tH - uint16(senVal);
                    coder.ceval('DrawTftLine', obj.x1, obj.y1, obj.x2,obj.y2, obj.Color1Index);
                    obj.y4 = tH - uint16(hpNew);
                    coder.ceval('DrawTftLine', obj.x1, obj.y3, obj.x2,obj.y4, obj.Color2Index);
                    obj.x1 = obj.x2;
                    obj.y1 = obj.y2;
                    obj.y3 = obj.y4;
                end
                obj.counter = obj.counter + 1;
            end
        end
  

        function [SenVal, HPVal] = getInputNamesImpl(~)
            SenVal = 'Raw Data';
            HPVal  = 'Filtered Data';
        end
        
        
        function icon = getIconImpl(~)
        % Define a string as the icon for the System block in Simulink.
        icon = 'ECG Signal Display';
        end
        
        
        function N = getNumInputsImpl(~)
            % Specify number of System inputs
            N = 2;
        end
        
        
        function N = getNumOutputsImpl(~)
            % Specify number of System outputs
            N = 0;
        end
        
        function flag = isInputSizeLockedImpl(~,~)
            flag = true;
        end
        
        function [SenVal, HPVal] = isInputFixedSizeImpl(~,~)
            SenVal = true;
            HPVal  = true;
        end
        
        function [SenVal, HPVal] = isInputComplexityLockedImpl(~,~)
            SenVal = true;
            HPVal  = true;
        end
    end

    methods(Static, Access = protected)
        %% Simulink customization functions
        function header = getHeaderImpl
            % Define header panel for System block dialog
            header = matlab.system.display.Header(...
                'ShowSourceLink', false, ...
                'Title', 'Display ECG Signal',... 
                'Text', 'Displays the current ECG signal onto the LCD screen');
        end
        
        function simMode = getSimulateUsingImpl
            % Return only allowed simulation mode in System block dialog
            simMode = 'Interpreted execution';
        end
        
        function flag = showSimulateUsingImpl
            % Return false if simulation mode hidden in System block dialog
            flag = false;
        end

        function groups = getPropertyGroupsImpl
            % Define property section(s) for System block dialog
            Line1Prop = matlab.system.display.internal.Property(...
                'line1Color_', 'Description', 'Line 1 Color');
            Line2Prop = matlab.system.display.internal.Property(...
                'line2Color_', 'Description', 'Line 2 Color');
            
            Group = matlab.system.display.Section(...
                'Title', 'Parameters', 'PropertyList', ...
                {Line1Prop,Line2Prop});
            
            groups = Group;
        end
    end
    methods (Static)
        function name = getDescriptiveName()
             name = 'ECG Signal Display';
        end
        
        function b = isSupportedContext(context)
            b = context.isCodeGenTarget('rtw');
        end
        
        function updateBuildInfo(buildInfo, context)
            % Update the build-time buildInfo
            if context.isCodeGenTarget('rtw')
                  spiRootDir = codertarget.arduinobase.internal.getArduinoIDERoot;
                  % Include Paths
                  addIncludePaths(buildInfo, fullfile(spiRootDir, 'hardware\arduino\avr\libraries\SPI\src'));
                  spkgRootDir = fileparts(strtok(mfilename('fullpath'), '+'));
                  % Include Paths
                  addIncludePaths(buildInfo, fullfile(spkgRootDir, 'include'));
                  addIncludeFiles(buildInfo, 'heart_monitor.h');
                  % Source Files
                  systemTargetFile = get_param(buildInfo.ModelName,'SystemTargetFile');
                  if isequal(systemTargetFile,'ert.tlc')
                    % Add the following when not in rapid-accel simulation
                    addSourcePaths(buildInfo, fullfile(spkgRootDir, 'include'));
                    addSourcePaths(buildInfo, fullfile(spiRootDir, 'hardware\arduino\avr\libraries\SPI\src'));
                    addSourceFiles(buildInfo, 'SPI.cpp', fullfile(spiRootDir, 'hardware\arduino\avr\libraries\SPI\src'), 'BlockModules');
                    addSourceFiles(buildInfo, 'Adafruit_ILI9341.cpp', fullfile(spkgRootDir, 'include'), 'BlockModules');
                    addSourceFiles(buildInfo, 'Adafruit_SPITFT.cpp', fullfile(spkgRootDir, 'include'), 'BlockModules');
                    addSourceFiles(buildInfo, 'Adafruit_GFX.cpp', fullfile(spkgRootDir, 'include'), 'BlockModules');
                    addSourceFiles(buildInfo, 'heart_monitor.cpp', fullfile(spkgRootDir, 'include'), 'BlockModules');
                  end     
            end           
        end
    end
end
