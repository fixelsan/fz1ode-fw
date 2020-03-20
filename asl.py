# ASL bootstrap
# (C) Felix Lazarev 2020


import serial
from time import sleep
import argparse

sectors=[
    0x0000,
    0x4000,
    0x8000,
    0xc000,
    0x10000,
    0x14000,
    0x18000,
    0x1c000,
    0x20000,
    0x40000,
    0x80000,
    0xc0000,
    0x100000,
    0x140000,
    0x180000,
    0x1c0000
]


def getsect(addr):
    sect=0
    for i in range(len(sectors)):
        if addr>=sectors[i]:
            sect=i
    return sect


def dumper(dat):
    ret=""
    for d in dat:
        ret+=hex(ord(d))+" "
    return ret


def le32(val):
    lns=""
    lns+=chr(val&0xff)
    lns+=chr((val>>8)&0xff)
    lns+=chr((val>>16)&0xff)
    lns+=chr((val>>24)&0xff)
    return lns


def main():
    global sectors
    parser = argparse.ArgumentParser(description="ASL Boot strapper")
    parser.add_argument('filename', metavar='filename', type=str, help='firmware image file')
    parser.add_argument('--flasher', type=str, default='', help='mandatory flasher binary')
    parser.add_argument('--port', type=str, default='', help='mandatory serial port')

    args = parser.parse_args()
    if args.flasher == '' or args.port == '':
        print ("Please check your arguments")
        parser.print_help()
        return
    try:
        flasher = open(args.flasher, "rb").read()
    except:
        print ("Unable to read flasher image")
        return

    try:
        firmware = open(args.filename, "rb").read()
    except:
        print ("Unable to read firmware image")
        return

    try:
        ser = serial.Serial(args.port,115200,rtscts=0,dsrdtr=0)
    except:
        print ("Unable to open serial port")
        return
    try:
        ser.write("\x01")
        sleep(0.05)
        ser.rts=0
        sleep(0.05)

        ser.write("\x00")
        dat=ser.read(1)
        if dat[0]!="\xd5":
            print ("ASL Header failed, please retry")
            ser.close()
            return
        ser.write(le32(len(flasher)))
        ser.flush()
        sleep(0.05)
        dat=ser.read(1)
        if dat[0]!="\x01":
            print ("ASL Length failed, please retry")
            ser.close()
            return
        ser.write(flasher)
        ser.flush()
        sleep(0.05)
        ser.timeout=5
        dat=ser.read(1)
        if dat[0]!="\x01":
            print ("ASL Transfer failed, please retry")
            ser.close()
            return
        sleep(0.05)
        dat=ser.read(1) #ignore tx setup
        ser.timeout=6
        dat=ser.read(3)
        print ("BOOT: "+dumper(dat))
        prot=dat[2]
        if prot=="\x01":
            print ("unprotecting ...")
            ser.write("\x04\x01\x02\x03\x04\x05\x06\x07\x08")
            dat=ser.read(1)
            print ("New protection status " + dumper(dat))
        sect=0
        addr=0
        ser.write("\x01"+le32(0x0c000000))
        ser.flush()
        print ("Addr res: "+ dumper(ser.read(1)))
        pg=""
        for byte in firmware:
            pg=pg+byte
            if len(pg)==256:
                waddr=0x0c000000+addr
                ws="\x02"+le32(waddr)+le32(256)+pg
                ser.write(ws)
                ser.flush()
                dt=ser.read(1)
                if dt[0]!="\x55":
                    print ("err @ addr " +hex(waddr))
                    ser.close()
                    return
                addr+=256
                ns=getsect(addr)
                if ns!=sect:
                    print("Erasing "+hex(addr))
                    ser.write("\x01"+le32(0x0c000000+addr))
                    ser.flush()
                    dt=ser.read(1)
                    if(dt[0]!="\x55"):
                        print ("Error on erase..." + dumper(dt))
                        ser.close()
                        return
                    sect=ns
                pg=""
        if len(pg)!=0:
             waddr=0x0c000000+addr
             ws="\x02"+le32(waddr)+le32(len(pg))+pg
             ser.write(ws)
             ser.flush()
             dt=ser.read(1)
             if(dt[0]!="\x55"):
                 print ("Error on erase..." + dumper(dt))
                 ser.close()
                 return

        print ("Protect")
        ser.write("\x03\x01\x02\x03\x04\x05\x06\x07\x08")
        dat=ser.read(1)
        print ("Protect result: "+dumper(dat))
        print ("Done")
        ser.close()
        return
    except:
        print ("Fault")
        return


if __name__=="__main__":
    main()
