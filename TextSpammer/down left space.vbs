'This sends txt via the sendkeys method to what txtarea has focus at the time Repeatedly until the variable 'mynum' is reached

set a = createobject("wscript.shell")

txt=" "
num=inputbox("How many times to send the text?","spamnumber","3") 
speed=inputbox("How fast to spam..In milisecs!","delay","200") 
wait=inputbox("Time to wait proir to sending the spam msg in secs","Wait?","5")
msgbox("You have " & wait & " secs to put focus on your target text area!")

wscript.sleep(wait*1000) 
for i=1  to num		'count down from mynum variable
	a.sendkeys(txt)       'Sends the text you typed in the mytxt prompt
	a.sendkeys("{DOWN}")   'presses the enter key to send your text you may change it to the apropriate key that sends the msg in your chat
	a.sendkeys("{LEFT}")
	wscript.sleep (speed)   'sleeps/waits the amount of milseconds you typed in the wait prompt
next
msgbox("Finished Spamming!")