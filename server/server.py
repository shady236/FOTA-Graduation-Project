from audioop import add
from ctypes.wintypes import MSG
from http import client, server
import os
import socket
import string
import threading
import time


ip      = '192.168.1.6'
port    = 800
ADD     = (ip,port)
format  = "utf-8"





server  = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(ADD)
server.listen(5)
print("server is listening...")


def updateConfigFile(ecuNum, latestAppVersion, latestBootVersion):
	configFile = open(f'ECU{ecuNum}/config.txt', 'w')
	configFile.write(str(latestAppVersion) + '\n')
	configFile.write(str(latestBootVersion))
	configFile.close()


def clientHandle(conn, add):
	global latestAppVersion, latestBootVersion
	
	upload   =  'U'
	download =  'D'

	app  =  'A'
	boot =  'B'
	
	ack  =  'A'
	nack =  'N'
	
	maxFileSize = 100000
	
	print("connect from {}".format(add))
	reqType = conn.recv(1).decode(format)
	ecuNum  = ''
	rxChar = conn.recv(1).decode(format)
	while True:
		try:
			x = int(rxChar)
			ecuNum += rxChar
			rxChar = conn.recv(1).decode(format)
		except:
			break
	
	if (reqType == upload):
		updateFor = rxChar
		updateContent1 = conn.recv(maxFileSize).decode(format)
		updateContent2 = conn.recv(maxFileSize).decode(format)
		
		latestAppVersion  = 0
		latestBootVersion = 0
		
		if os.path.exists(f'ECU{ecuNum}'):
			configFile = open(f'ECU{ecuNum}/config.txt', 'r')
			latestAppVersion  = int(configFile.readline())
			latestBootVersion = int(configFile.readline())
			configFile.close()
		else:
			os.makedirs(f'ECU{ecuNum}')
			configFile = open(f'ECU{ecuNum}/config.txt', 'w')
			configFile.write('0\n')
			configFile.write('0')
			configFile.close()
			
		if (updateFor == app):
			latestAppVersion += 1
			newAppRegion1 = open(f"ECU{ecuNum}/version{latestAppVersion}App1.srec", 'w')
			newAppRegion1.write(updateContent1)
			newAppRegion1.close()
			newAppRegion2 = open(f"ECU{ecuNum}/version{latestAppVersion}App2.srec", 'w')
			newAppRegion2.write(updateContent2)
			newAppRegion2.close()
		elif (updateFor == boot):
			latestBootVersion += 1
			newBootRegion1 = open(f"ECU{ecuNum}/version{latestBootVersion}Boot1.srec", 'w')
			newBootRegion1.write(updateContent1)
			newBootRegion1.close()
			newBootRegion2 = open(f"ECU{ecuNum}/version{latestBootVersion}Boot2.srec", 'w')
			newBootRegion2.write(updateContent2)
			newBootRegion2.close()
		updateConfigFile(ecuNum, latestAppVersion, latestBootVersion)
		
	elif (reqType == download):
		requiredFile = rxChar
		requiredRegion = int(conn.recv(1).decode(format))
		
		latestAppVersion  = 0
		latestBootVersion = 0
		
		if os.path.exists(f'ECU{ecuNum}'):
			configFile = open(f'ECU{ecuNum}/config.txt', 'r')
			latestAppVersion  = int(configFile.readline())
			latestBootVersion = int(configFile.readline())
			configFile.close()
		
		if (requiredFile == app):
			print(f'client requested App for ECU {ecuNum}')
			conn.send(str(latestAppVersion).encode(format))
		elif (requiredFile == boot):
			print(f'client requested Boot for ECU {ecuNum}')
			conn.send(str(latestBootVersion).encode(format))
		
		responce = conn.recv(1).decode(format)
		if (responce == ack):
			if (requiredFile == app):
				print(f'client is reciving App version {latestAppVersion} for region {requiredRegion}', end='')
				f = open(f"ECU{ecuNum}/version{latestAppVersion}App{requiredRegion}.srec", 'r')
				for line in f:
					conn.send(line.encode(format))
					responce = conn.recv(1).decode(format)
					if (responce == nack):
						print('\nfailed to update App for {}'.format(add))
						break
				f.close()
				if (responce == ack):
					print('\nsuccessfully updated App for {}'.format(add))
					
			elif (requiredFile == boot):
				print(f'client is reciving Bootloader version {latestBootVersion} for region {requiredRegion}', end='')
				f = open(f"ECU{ecuNum}/version{latestBootVersion}Boot{requiredRegion}.srec", 'r')
				for line in f:
					conn.send(line.encode(format))
					responce = conn.recv(1).decode(format)
					if (responce == nack):
						print('\nfailed to update Bootloader for {}'.format(add))
						break
				f.close()
				if (responce == ack):
					print('\nsuccessfully updated Bootloader for {}'.format(add))
					
	conn.close()


while True:
	conn, add = server.accept()
	thread = threading.Thread(target=clientHandle, args=(conn, add))
	thread.start()
	