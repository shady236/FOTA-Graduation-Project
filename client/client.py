import os
import socket
import threading
import time
from tkinter import *
from tkinter import ttk
from tkinter import filedialog
import signal


app      =  'A'
boot     =  'B'


def msg(msgTitle, msgText):
	msgFrame = Toplevel()
	msgFrame.title(msgTitle)
	msgFrame.minsize(max(len(msgText) * 9, 300), 100)
	msgFrame.resizable(False, False)
	
	errorMsgLabel = Label(msgFrame, text='\n' + msgText, font=('Arial', 11))
	errorMsgLabel.pack()
	

def uploadThread(fileType, ecuNum, file1Path, file2Path):
	try:
		serverPort = 800
		serverIP   = "192.168.1.6"
		serverAddress = (serverIP, serverPort)
		format  = "utf-8"

		upload   =  'U'
		client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)    # create a socket object           
		client.connect(serverAddress)    # connect to the server using its Address (IP, port)
		client.send(upload.encode(format))    # send upload request to the server
		client.send(ecuNum.encode(format))    # send ECU number to the server
		
		if (fileType == app):
			file1 = open(file1Path, 'r')
			file2 = open(file2Path, 'r')
		elif (fileType == boot):
			file1 = open(file1Path, 'r')
			file2 = open(file2Path, 'r')
		
		client.send(fileType.encode(format))    # send type of the file type
		
		client.send(file1.read().encode(format))    # send the file to the server
		time.sleep(3)
		client.send(file2.read().encode(format))    # send the file to the server
		file1.close()
		file2.close()

		client.close()
		
		msg('Upload', 'Upload successful')
		
	except:
		msg('No internet connection', 'Please check your internet connection')




def submitInfo(fileType, ecuNum, file1Path, file2Path):
	
	# if the ecuNum is not a number, then it is not a valid ecu number
	try:
		x = int(ecuNum)
		if x < 0:
			msg('Invalid ECU number', 'Please enter a valid ECU number')
			return
	except:
		msg('Invalid ECU number', 'Please enter a valid ECU number')
		return
	
	# if any of the files isn't selected, print error message
	if file1Path == '' or file2Path == '':
		msg('Missing Data', 'Please select both two files')
		return
	
	# if the files are the same, print error message
	if file1Path == file2Path:
		msg('Same File', 'Please select two different files')
		return
	
	# if any of the files isn't a .srec file, print error message
	if not file1Path.endswith('.srec') or not file2Path.endswith('.srec'):
		msg('Invalid File', '.srec files are only accepted')
		return
	
	# if files not exist, print error message
	if not os.path.isfile(file1Path) or not os.path.isfile(file2Path):
		msg('File Not Found', 'Please select valid file paths')
		return
	
	# all is correct, start uploading in new thread
	thread = threading.Thread(target=uploadThread, args=(fileType, ecuNum, file1Path, file2Path))
	thread.start()
	
	


def openBrowsingWindow(fileNum):
	global file1Path, file2Path
	
	if fileNum == 1:
		file1Path.delete(0, END)
		file1Path.insert(0, filedialog.askopenfilename())
	elif fileNum == 2:
		file2Path.delete(0, END)
		file2Path.insert(0, filedialog.askopenfilename())


def updateFileType(newFileType):
	global fileType, file1Label, file2Label
	
	fileType.set(newFileType)
	
	if newFileType == app:
		file1Label.config(text='Application 1 file path')
		file2Label.config(text='Application 2 file path')
	elif newFileType == boot:
		file1Label.config(text='Bootloader 1 file path')
		file2Label.config(text='Bootloader 2 file path')



# run GUI
root = Tk()
root.title('FOTA Upload Center')
root.minsize(420, 450)
root.resizable(False, False)

# add margins to the window
frame = Frame(root)
frame.pack(fill = 'both', expand = True, padx = 10, pady = 10)


regularFont = ('calibri', 13, 'normal')
boldFont = ('calibri', 10, 'bold')
bigFont  = ('calibri', 20, 'bold')


curRow = 0

# radio buttons for choosing the file type either app or boot
fileTypeLabel = Label(frame, text = 'Update Type:', font = regularFont)
fileTypeLabel.grid(row = curRow, column = 0, sticky = W)
curRow += 1
fileType = StringVar(root, app)

rb = ttk.Radiobutton(frame, text="Application", variable=fileType, value=app, command=lambda: updateFileType(app))
rb.grid(row = curRow, column = 0, sticky = W)
curRow += 1
rb = ttk.Radiobutton(frame, text="Bootloader", variable=fileType, value=boot, command=lambda: updateFileType(boot))
rb.grid(row = curRow, column = 0, sticky = W)
curRow += 1




# insert some padding
Label(frame, text='', font=regularFont).grid(row=curRow)
curRow += 1


# create a label & entry for ECU number
ecuNumLabel = Label(frame, text='ECU Number', font=regularFont)
ecuNumLabel.grid(row=curRow, column=0, sticky=W)
curRow += 1
ecuNumBox = Entry(frame, width=40, font=regularFont)
ecuNumBox.grid(row=curRow, sticky=W)
curRow += 1



# insert some padding
Label(frame, text='', font=regularFont).grid(row=curRow)
curRow += 1



# create a label & entry for file1 path
file1Label = Label(frame, text='Application 1 file path', font=regularFont)
file1Label.grid(row=curRow, column=0, sticky=W)
curRow += 1
file1Path = Entry(frame, width=40, font=regularFont)
file1Path.grid(row=curRow, sticky=W)
file1BrowseBtn = Button(frame, text='. . .', command=lambda: openBrowsingWindow(1), bg='grey80', font=boldFont)
file1BrowseBtn.grid(row=curRow, column=1, sticky=W)
curRow += 1




# insert some padding
Label(frame, text='', font=regularFont).grid(row=curRow)
curRow += 1


# create a label & entry for file2 path
file2Label = Label(frame, text='Application 2 file path', font=regularFont)
file2Label.grid(row=curRow, column=0, sticky=W)
curRow += 2
file2Path = Entry(frame, width=40, font=regularFont)
file2Path.grid(row=curRow, sticky=W)
file2BrowseBtn = Button(frame, text='. . .', command=lambda: openBrowsingWindow(2), bg='grey80', font=boldFont)
file2BrowseBtn.grid(row=curRow, column=1, sticky=W)
curRow += 1



# insert big padding
Label(frame, text='', font=bigFont).grid(row=curRow)
curRow += 1



# create a button to submit the info and start downloading
uploadBtn = Button(frame, text='Start Uploading', command=lambda: submitInfo(fileType.get(), ecuNumBox.get(), file1Path.get(), file2Path.get()), width=40//2, bg='green', fg='white', font=bigFont)
uploadBtn.grid(row=curRow, sticky=S)


root.mainloop()