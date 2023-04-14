; Press Caps Lock to toggle Mouse Keys on/off

spd := 12

CoordMode, Mouse, Screen

SysGet, MonitorCount, MonitorCount

Loop %MonitorCount%
{
    SysGet, Monitor, Monitor, %A_Index%
    MsgBox, Monitor %A_Index%:`nWidth: %MonitorRight%`nHeight: %MonitorBottom%
}

CapsLock::
    if (MouseKeysActive)
    {
        MouseKeysActive := false
        SetMouseSpeed(spd) 
    }
    else
    {
        MouseKeysActive := true
        SetMouseSpeed(0) ; Set mouse speed to 0 to disable acceleration
    }
return


; Use the numpad keys to move the mouse cursor
Numpad1::MouseMove, -spd, spd, 0, R, {Blind} ; Move cursor left and down
Numpad2::MouseMove, 0, spd, 0, R, {Blind}  ; Move cursor down
Numpad3::MouseMove, spd, spd, 0, R, {Blind}   ; Move cursor right and down
Numpad4::MouseMove, -spd, 0, 0, R, {Blind}  ; Move cursor left

NumpadEnter::
    spd := 34
	SetMouseSpeed(spd) 
return

NumpadEnter Up::
    spd := 12
	SetMouseSpeed(spd) 
return

Numpad5::
    Click down left
return

Numpad5 Up::
    Click up left
return

Numpad6::MouseMove, spd, 0, 0, R, {Blind}   ; Move cursor right
Numpad7::MouseMove, -spd, -spd, 0, R, {Blind} ; Move cursor left and up
Numpad8::MouseMove, 0, -spd, 0, R, {Blind}  ; Move cursor up
Numpad9::MouseMove, spd, -spd, 0, R, {Blind}  ; Move cursor right and up

; Function to set the mouse speed
SetMouseSpeed(speed)
{
    DllCall("SystemParametersInfo", UInt, 113, UInt, 0, UIntP, speed, UInt, 0)
}

; Initialize the script
MouseKeysActive := false
SetMouseSpeed(spd) ; Set default mouse speed