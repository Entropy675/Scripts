Loop
{
    MouseMove, -50, 0, 10, R  ; move mouse left
    Sleep, 50  ; wait for 50 milliseconds
    MouseMove, 50, 0, 10, R  ; move mouse right
    Sleep, 50  ; wait for 50 milliseconds
    if GetKeyState("Escape", "P")  ; check if Escape key is pressed
        break  ; exit loop if Escape key is pressed
}