
'''
TODO
##DONE
    ##make the map depend on screen resolution
    ##export a png or jpg image of the map
    ##read from a file if there were previous mines to add them to the map
    ##save data to a file
'''

#Tested with Python 3.6.4 32-bit

#Requirnments:
#   Download pip to be able to install the libraries
#   Download the following libraries:
#    Pillow
#    canvasvg
#    cairosvg
#    svglib
#    pyserial

#Last tested 3 August 2018

from turtle import * #the library used for drawing the map
import tkinter as tk #the library used to caontrol the window where the map is drawn
from PIL import Image #Used to manipulate the image of the map 
import canvasvg #to export svg(extension of a photo file) image file of the map
#import cairosvg
from svglib.svglib import svg2rlg
from reportlab.graphics import renderPDF ,renderPM
import serial #Reads the data from the serial
import time #used for delay

mines = {}  #the main list where all the data of the mines is stored
index = 0 #mine number
a = []
data = []
x = 1

ser = serial.Serial('COM12' , 9600) #making a serial object named ser with COM19 and baudrate of 38400
#change them according to your ardiuno

def serialCom():    #function used to establish a connection between the arduino with a handshake
    print("Establishing serial connection")
    print("Sending start byte..")
    time.sleep(5)   #waits 5 seconds to make sure that the arduino started working
    ser.write(b"A") #writes a byte to the serial , A
    time.sleep(1)
    while ser.readline().decode("utf-8").strip("\r\n") != 'A':  #recieves the byte sent by the arduino and performs a chain of commands
        #readline() reads the data from the serial
        #decode()   decodes the comming data from bytes to string
        #strip()    removes any \r and \n from the recieved data
        #thus the received data will be A
        print("Sending start byte..")
        print(ser.readline().decode("utf-8").strip("\r\n"))
        ser.write(b"A") #keeps writing byte A to the serial until it reads A then it stops and move to receiving mines data
        time.sleep(0.5)
    print("Start byte recieved")
    print("Reading mines")
    receiveData()   #when the A is received(handshaking is done) ,it will jump to receiveData() function

def receiveData():#this is the main funtion of the code where it works on receiving the data of the mines
    print("Recieving Data..")
    print("Byte Received")
    time.sleep(0.5)
    
    x = 0  #a counter used in the if condition below
    c = 0  #a couter used as an index in the if condition when calling mark_mine() function
    tempData = []#A temporary list to store the data of the received mine
    
    while True:
        rdata = ser.readline().decode("utf-8").strip("\r\n") #stores the received number in rdata ,removing all the \r and \n
        mark_mine([float(rdata.split(';')[0]),float(rdata.split(';')[1]),float(rdata.split(';')[2]),(rdata.split(';')[3])])
##        if(rdata == '#'):
##            while(1):
##                ser.write(b"#")
##                rdata = ser.readline().decode("utf-8").strip("\r\n")
##                print(rdata)
##                tempData.append(rdata)
##                if(len(tempData) == 4):
##                    print(tempData)
##                    mark_mine(tempData)
##                    tempData = []
##                    #x -= 1
##                    break
        print(rdata)
##        tempData.append((rdata)) #this line adds the received number(rdata) to the last place in the tempData list
##
##        if x % 2 == 0 and x is not 0: #this if condition is made to plot the mines on the map one by one
##            #when x % 2 == 0 , meaning the number of received numbers is 2
##            #and x is not 0 , meaning it is not the first number to receive
##            #then do the following..
##            
##            data.append(tempData) #append the tempData list(carrying the data of the current mine) in the data list(the main list)
##            mark_mine(data[c])  #call mark_mine() function with the data of the current mine (data[c]) ,marks the mine on the map
##            #don't read mark_mine() funtion now. ^^
##            print("----------")
##            c += 1 #increase the index with 1 so that the next time it marks the mine, it'll send the second list in the data list
##            #then the third and so on..
##            tempData = [] #clears tempData list from everything
##            x = -1
##            
##        x += 1 #this line and the one above resets the x to 0

##        try :
##            if(int(rdata) == 7):
##                save_image("bl")
##                break
##        except ValueError:
##            print(rdata)
        
    
def read_mines():
    while ser.readline().decode("utf-8").strip("\r\n") != "H":
        a.append(ser.readline().decode("utf-8").strip("\r\n"))

def screen():#this funtion sets the dinemsions of the window of the map
    root = tk.Tk()
    screen_width = root.winfo_screenwidth() #gets the width of your laptop's screen
    screen_height = root.winfo_screenheight()   #gets the length of your laptop's screen
    width = int(screen_width)*0.75 #dimesions of the map
    length = int(screen_height)*0.75 #i've divided by two ,you can change it as you wish
    root.destroy()  
    print('Screen Resolution {} x {}'.format(screen_height ,screen_width))
    setup(700 , 700 , 0 , 0) #Screen size
    return width ,length
    
def draw_map(R , C , width , length):#draws the grid map
    print('Map Resolution {} x {}'.format(length ,width))
    title('MAP') #sets the name of the window
    print('Drawing Map..')
    speed(0) #speed of cursor ,change it if you want
    pu() #don't draw ,penup
    goto (-width / 2, length / 2 ) #goto upper corner of the screen ,moves the cursor
    Sqx = width / R #width of the Square proportional with the width and length of the map
    Sqy = length / C #length of the Square
    
    for y in range(0, C ):#Vertical
        for x in range(0 ,R):#Horizontal
            pd() #draw
            goto(xcor () , ycor() - Sqy )
            goto(xcor () , ycor() + Sqy )
            goto(xcor() + Sqx , ycor() )
        goto(xcor() , ycor() - Sqy )
        goto(-width/2 , ycor())
    #the above nested for loop draws the grid map by moving the cursor to certain positions
    print("Map is drawn")


def starting_point(starting_corner = 'br'):#set starting point of the robot
    D = {'ul' : [-width/2 , length/2] , 'ur' : [width/2 , length/2] , 'bl' : [-width/2 , -length/2] , 'br' : [width/2 , -length/2]}
    pu()
    goto(D[starting_corner][0] , D[starting_corner][1])

def mark_mine(mine):#place mine on the map
    mineX = mine[0]
    mineY = mine[1]
    if(mineY == 0):
        return
    S = 0#sign X
    #print(mineX,mineY)
    H = 0#sign Y
    place = int(mine[2])
    Sqx = 600 / 20 #width of the Square
    Sqy = 600 / 20 #length of the Square
    pu()
    if S == 0:
        try:
            goto(600/2 + ((-mineX)/100)*Sqx , -600/2 + ((mineY)/100)*Sqy)
        except ValueError:
            print("Over flow")
    else:
        if mineX == "0.00":
            mineX = 0
        goto(-600/2 + (float(mineX)/100)*Sqx , -600/2 + (float(mineY)/100)*Sqy)
##      print(mine)
        
    if place == 1:
        #under ground
        begin_fill()
        color("orange")
        X = ((9 - int(mineX/100)) * Sqx)
        Y = -((10 - int(mineY/100)) * Sqy)
        goto(X,Y)
        goto(X + 30,Y)
        goto(X + 30,Y + 30)
        goto(X,Y + 30)
        goto(X,Y)
        end_fill()
    
    elif place == 0:#above ground
        begin_fill()
        color("blue")
        X = ((9 - int(mineX/100)) * Sqx)
        Y = -((10 - int(mineY/100)) * Sqy)
        goto(X,Y)
        goto(X + 30,Y)
        goto(X + 30,Y + 30)
        goto(X,Y + 30)
        goto(X,Y)
        end_fill()

    elif int(place) == 2:#robot tracking
        begin_fill()
        color("black")
        circle(2)
        end_fill()
        
def save_mine(mineX , mineY):
    mines[str(index)] = [mineX , mineY]
    f = open('MINES.txt' , 'a')
    f.write('MINE {} at X: {} , Y: {}\n'.format(index , mineX , mineY))
    f.close()

def read_file(name):
    print('Reading mines from {} .'.format(name))
    o = open(name , 'r')
    data = o.read().split()
    print(len(data)/8)
    print(data)
    i = 4
    for x in range (0,int(len(data)/8)):
        
        mark_mine([ int(data[i]) ,int(data[i + 3]),int(data[i - 3]) ] )
        i += 8
    o.close()
    print('Mines added.')
    
def save_image(Bcorner):
    print("Drawing the image.")
    if Bcorner not in ['bl' ,'br' ,"ul" ,'ur']:
        print('you entered {} ,it was set to bl.'.format(Bcorner))
    ts = getscreen().getcanvas()
    canvasvg.saveall("MAP.svg" ,ts)
    #cairosvg.svg2png(url = "MAP.svg" , write_to = "MAP.png")
    drawing = svg2rlg("MAP.svg")
    renderPM.drawToFile(drawing ,"MAP.png")
    img = Image.open("MAP.png")
    
    if Bcorner == 'bl': 
        img = img
    elif Bcorner == 'br':
        img = img.transpose(Image.FLIP_LEFT_RIGHT)
    elif Bcorner == 'ul':
        img = img.transpose(Image.FLIP_TOP_BOTTOM)
    elif Bcorner == 'ur':
        img = img.transpose(Image.FLIP_LEFT_RIGHT)
        img = img.transpose(Image.FLIP_TOP_BOTTOM)

    img.save('MAPNEW.png')
    img.show()
    i = input('Exit.')



width , length = screen()   #calls screen() function and stores the two returned numbers in width and length variables
draw_map(20,20,600 , 600)  #calls draw_map() funtion with 4 parameters , number of rows , number of coloumns , width and length of the map

if input('Do you want to use older file ? [Y/n]') == 'Y':
    name = input('name the file without (.txt) ')
    read_file(str(name) + '.txt')
else:
    print('OK')
##starting_corner = input("where do you want to start from  (ul , ur , bl , br) \nbl (Bottom left) is default. ")
##starting_point(starting_corner)

print("Detecting mines.")
serialCom()
