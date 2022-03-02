import serial
import struct
import time
import sys

def GBA_Send32(val):
    val = int(val) & 0xFFFFFFFF

    send = struct.pack("<LL", 0, val);
    written = 0
    while written != len(send):
        written = ser.write(send)
        ser.flush()

    data = ser.read(8)
    if len(data) != 8:
        print("No response from ST Link...? Retrying...")
        return GBA_Send32(val)

    cmd,val = struct.unpack("<LL", data);

    return val

def GBA_Send32Fast(val):

    val = int(val) & 0xFFFFFFFF

    send = struct.pack("<LL", 1, val);
    written = 0
    while written != len(send):
        written = ser.write(send)
        ser.flush()

    return 0xFFFFFFFF

def GBA_Send32NotBios(val):

    val = int(val) & 0xFFFFFFFF

    send = struct.pack("<LL", 3, val);
    written = 0
    while written != len(send):
        written = ser.write(send)
        ser.flush()

    data = ser.read(8)
    if len(data) != 8:
        print("No response from ST Link...? Retrying...")
        return GBA_Send32(val)

    cmd,val = struct.unpack("<LL", data);

    return val

def GBA_Send8(val):
    val = int(val) & 0xFF

    send = struct.pack("<LL", 4, val);
    written = 0
    while written != len(send):
        written = ser.write(send)
        ser.flush()

    data = ser.read(8)
    if len(data) != 8:
        print("No response from ST Link...? Retrying...")
        return GBA_Send32(val)

    cmd,val = struct.unpack("<LL", data);

    return val

def GBA_Send8Fast(val):

    val = int(val) & 0xFF

    send = struct.pack("<LL", 5, val);
    written = 0
    while written != len(send):
        written = ser.write(send)
        ser.flush()

    return 0xFFFFFFFF

def GBA_Send8NotBios(val):

    val = int(val) & 0xFF

    send = struct.pack("<LL", 6, val);
    written = 0
    while written != len(send):
        written = ser.write(send)
        ser.flush()

    data = ser.read(8)
    if len(data) != 8:
        print("No response from ST Link...? Retrying...")
        return GBA_Send32(val)

    cmd,val = struct.unpack("<LL", data);

    return val

def LED(val):
    val = int(val) & 0xFFFFFFFF

    send = struct.pack("<LL", 2, val);
    ser.write(send)

def wait_mb():
    for i in range(0,1000):
        resp = GBA_Send32(0xF00F8765)
        if resp == 0xF00F1234:
            return resp

    resp = 0
    while True:
        resp = GBA_Send32(0x00006202)
        if resp == 0x72026202:
            return resp
        
    return resp

if len(sys.argv) < 4:
    print ("Usage: upload.py /dev/tty.usbmodemWhatever your_file_to_send_mb.gba your_file_to_send_2_mb.gba")
    exit(-1)

port = sys.argv[1]
to_send = sys.argv[2]

ser = serial.Serial(timeout=0.1)
ser.port=port
ser.open()

def do_mb():
    GBbuffer = open(to_send, "rb").read()
    GBbuffer_length = len(GBbuffer)
    print ("Sending",hex(GBbuffer_length),"bytes")
    read_from_gba = 0
    fp = 0
    bit = 0
    fcnt = 0
    mbfsize = GBbuffer_length - 0xC0

    if wait_mb() == 0xF00F1234:
        return True

    #LED(1)

    read_from_gba = GBA_Send32(0x00006202);
    read_from_gba = GBA_Send32(0x00006102);

    fp=0;
    for i in range(0, 32):
        w = GBbuffer[fp];
        w = GBbuffer[fp+1] << 8 | w;
        fcnt += 2;
        fp += 2;
        r = GBA_Send32(w);


    for i in range(0, 32):
        w = GBbuffer[fp];
        w = GBbuffer[fp+1] << 8 | w;
        fcnt += 2;
        fp += 2;
        r = GBA_Send32(w);

    old_size = mbfsize
    mbfsize = (mbfsize+0x0f)&0xfffffff0;    #align length to xfer to 16

    for i in range(0, mbfsize - old_size):
        GBbuffer += b'\x00'

    for i in range(0, 32):
        w = GBbuffer[fp];
        w = GBbuffer[fp+1] << 8 | w;
        fcnt += 2;
        fp += 2;
        r = GBA_Send32(w);



    r = GBA_Send32(0x00006200);
    r = GBA_Send32(0x00006202);
    r = GBA_Send32(0x000063d1);
    r = GBA_Send32(0x000063d1);
    m = ((r & 0x00ff0000) >>  8) + 0xffff00d1;
    h = ((r & 0x00ff0000) >> 16) + 0xf;
    r = GBA_Send32((((r >> 16) + 0xf) & 0xff) | 0x00006400);
    r = GBA_Send32((mbfsize - 0x190) // 4);
    f = (((r & 0x00ff0000) >> 8) + h) | 0xffff0000;
    c = 0x0000c387;

    time.sleep(0.016)

    led_state = 0;

    fp=0xC0;
    while(fcnt < mbfsize):
        if (fp % 0x1000 == 0):
            led_state = (0 if led_state == 1 else 1)
            print ("sending...", hex(fp))
            LED(led_state)

        w = GBbuffer[fp];
        w = GBbuffer[fp+1] <<  8 | w;
        w = GBbuffer[fp+2] << 16 | w;
        w = GBbuffer[fp+3] << 24 | w;
        fp += 4;
        w2 = w;
        for bit in range(0, 32):
            if((c ^ w) & 0x01):
                c = (c >> 1) ^ 0x0000c37b;
            else:
                c = c >> 1;
            w = w >> 1;
        m = (0x6f646573 * m) + 1;
        if (fp % 0x1000 == 0):
            read_from_gba = GBA_Send32(w2 ^ ((~(0x02000000 + fcnt)) + 1) ^m ^0x43202f2f)
        else:
            read_from_gba = GBA_Send32Fast(w2 ^ ((~(0x02000000 + fcnt)) + 1) ^m ^0x43202f2f)

        if read_from_gba != 0xFFFFFFFF and (read_from_gba >> 16) != fcnt:
            print ("This multiboot probably failed: received", hex(read_from_gba))
            time.sleep(1)
            return False

        fcnt = fcnt + 4;

    test_delay = True
    LED(1);


    for bit in range(0, 32):
        if((c ^ f) & 0x01):
            c =( c >> 1) ^ 0x0000c37b;
        else:
            c = c >> 1;
        f = f >> 1;

    read_from_gba = 0
    while(read_from_gba!=0x00750065):
        read_from_gba = GBA_Send32(0x00000065)

    r = GBA_Send32(0x00000066);
    r = GBA_Send32(c);

    if r & 0xFF != 0x66 or (r >> 16) != c:
        print ("CRC mismatch!", hex(r))
        return False

    LED(0)
    return True

# Keep trying to send until it succeeds
while True:
    transferred = True#do_mb()
    if not transferred:
        print("Failed? Retrying...")
        continue

    break

print ("Sending next payload...")
while True:
    time.sleep(0.016)
    v = GBA_Send32NotBios(0xF00F5678)
    if v != 0xFFFFFFFF:
        print (hex(v))
    if v == 0xF00F1234:
        break

to_send = sys.argv[3]
GBbuffer = open(to_send, "rb").read()
GBbuffer += (0x10 - (len(GBbuffer) & 0xF)) * b"\x00"
GBbuffer_length = len(GBbuffer)
sent_bytes = 0
print ("Sending",hex(GBbuffer_length),"bytes")

print(hex(GBA_Send32NotBios(GBbuffer_length)))

while True:
    v = GBA_Send32NotBios(struct.unpack("<L", GBbuffer[sent_bytes:sent_bytes+4])[0])
    sent_bytes = v - 0x08000000 + 4
    #print (hex(v), hex(GBbuffer_length), hex(sent_bytes))
    if (sent_bytes % 0x1000 == 0):
        print (hex(sent_bytes))
    if sent_bytes >= GBbuffer_length:
        break

print ("Done. Console time:")

buffer = ""
def gba_readstr():
    global buffer
    time.sleep(0.016)
    v = GBA_Send32NotBios(ord('a'))
    if v & 0xFFFFFF00 == 0 and v != 0xFFFFFFFF and v & 0xFF != 0xFF:
        c = chr(v)
        if (c == '\n'):
            print (buffer)
            buffer = ""
        else:
            buffer += c

def gba_writestr(outval):
    global buffer
    for out_c in outval:
        #time.sleep(0.016)
        time.sleep(0.001)
        v = GBA_Send32NotBios(ord(out_c))
        if v & 0xFFFFFF00 == 0 and v != 0xFFFFFFFF and v & 0xFF != 0xFF:
            c = chr(v)
            if (c == '\n'):
                print (buffer)
                buffer = ""
            else:
                buffer += c
#time.sleep(5)
while True:
    v = input()
    gba_writestr(v + "\n")
