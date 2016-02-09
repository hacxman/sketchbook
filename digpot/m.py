import math
import numpy as np

w0 = [int(127*math.sin(2*math.pi*t/256.0)) for t in range(256)]
w1 = [int(127*(math.cos(3*math.pi*t/256.0)*0.5+math.sin(8*math.pi*t/256.0)*0.3)) for t in range(256)]

w0f = np.fft.rfft(w0)
w1f = np.fft.rfft(w1)

#print w0f, w1f
maxt = 32.0
wf = []
for i in range(int(maxt)):
  t = i/maxt
  #print t
  s = map(lambda (x,y): (x*(1-t)+y*t), zip(w0f, w1f))
  d = np.fft.irfft(s)
  #print d
  wf.append(d)


def gen_c_header(fil):
  print>>fil, "const int samplelenght =", len(w0), ';'
  print>>fil, "const int samplecount =", int(maxt), ';'
  print>>fil, "const char wavetables[] PROGMEM = {"
  for i, w in enumerate(wf):
    s = ', '.join(map(str, map(int, w)))
    print>>fil, s, ','
  print>>fil, "};"

import sys, os
f = sys.stdout
if len(sys.argv) > 1:
  f = open(sys.argv[1], 'w+')
gen_c_header(f)
