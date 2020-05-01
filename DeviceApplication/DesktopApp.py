from tkinter import *
import serial

class GUI(Frame):

    def __init__(self,master=None):
        Frame.__init__(self, master)

        self.grid()
        self.winfo_toplevel().title("Modufi Desktop App")

        self.connectionCheck()

    def connectionCheck(self, master=None):
        color = boardComms()
        if color == "green":
            connected_button1 = Button(master, text = 'Connected', fg="#147a24").grid(row = 0)
            connected_button2 = Button(master, text = 'Not Connected', state="disabled").grid(row = 0, column = 1)

            self.nwLabel = Label(master, text="Network Name").grid(row = 1)

            self.nwEntry = StringVar()
            self.nwEntry = Entry(textvariable=self.nwEntry, state="disabled")
            self.nwEntry.grid(row = 1, column = 1)

            self.pwLabel = Label(master, text="Network Password").grid(row = 2)

            self.pwEntry = StringVar()
            self.pwEntry = Entry(textvariable=self.pwEntry)
            self.pwEntry.grid(row = 2, column = 1)

            self.eventLabel = Label(master, text="Event Name").grid(row = 3)

            self.eventEntry = StringVar()
            self.eventEntry = Entry(textvariable=self.eventEntry)
            self.eventEntry.grid(row = 3, column = 1)

            self.keyLabel = Label(master, text="Event Key").grid(row = 4)

            self.keyEntry = StringVar()
            self.keyEntry = Entry(textvariable=self.keyEntry)
            self.keyEntry.grid(row = 4, column = 1)

            self.submitButton = Button(master, text="Submit", command=self.submitButtonClick)
            self.submitButton.grid(row = 5, column = 1)

        else:
            connected_button1 = Button(master, text = 'Connected', state="disabled").grid(row = 0)
            connected_button2 = Button(master, text = 'Not Connected', fg = "#af4242").grid(row = 0, column = 1)

            self.nwLabel = Label(master, text="Network Name", state="disabled").grid(row = 1)

            self.nwEntry = StringVar()
            self.nwEntry = Entry(textvariable=self.nwEntry, state="disabled")
            self.nwEntry.grid(row = 1, column = 1)

            self.pwLabel = Label(master, text="Network Password", state="disabled").grid(row = 2)

            self.pwEntry = StringVar()
            self.pwEntry = Entry(textvariable=self.pwEntry, state="disabled")
            self.pwEntry.grid(row = 2, column = 1)

            self.eventLabel = Label(master, text="Event Name", state="disabled").grid(row = 3)

            self.eventEntry = StringVar()
            self.eventEntry = Entry(textvariable=self.eventEntry, state="disabled")
            self.eventEntry.grid(row = 3, column = 1)

            self.keyLabel = Label(master, text="Event Key", state="disabled").grid(row = 4)

            self.keyEntry = StringVar()
            self.keyEntry = Entry(textvariable=self.keyEntry, state="disabled")
            self.keyEntry.grid(row = 4, column = 1)

            self.submitButton = Button(master, text="Submit", state="disabled")
            self.submitButton.grid(row = 5, column = 1)

        self.submitButton = Button(master, text="Refresh Connection", command=self.refreshButton)
        self.submitButton.grid(row = 6, column = 1)

    def networkName(self):
        return network_name

    def networkPW(self):
        return network_pw

    def eventName(self):
        return event_name

    def keyName(self):
        return key_name

    def submitButtonClick(self):
                print(self.nwEntry.get())
                global network_name 
                global network_pw
                network_name = self.nwEntry.get()
                network_pw = self.pwEntry.get()
                event_name = eventEntry
                key_name = keyEntry

                try:
                    nw_name = self.networkName()
                    nw_password = self.networkPW()
                    ev_name =  self.eventName()
                    ky_name = self.keyName()

                except:
                    print("error: Fields not inputted correctly")

    def refreshButton(self):
        color = boardComms()
        self.connectionCheck()
        print("refreshed")
        print(color)

def boardComms():
    try:
        ser = serial.Serial('/dev/tty.usbserial-DN02BENB', 57600)
        color = "green"
    except:
        color = "red"

    return color

guiFrame = GUI()    
guiFrame.mainloop()





