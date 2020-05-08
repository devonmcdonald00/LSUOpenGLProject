import sys
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator

import math
import csv

x = []
y = []

i = 0
with open('collapse_analysis.csv') as csvfile:
	plots = csv.reader(csvfile, delimiter=',')
	for row in plots:
		if i != 0:
			x.append(int(row[1]))
			y.append(int(row[0]))
		i+=1


##ax = plt.figure().gca()
##ax.xaxis.set_major_locator(MaxNLocator(integer=True))
plt.plot(x, y, marker = 'o')

#if sys.argv[1] == 'individual':
#	plt.title('Average Faces Collapsed per Second vs N Measurements')
#	plt.xlabel('Measurement N')
#	plt.ylabel('Average Faces Collapsed per Second')

#if sys.argv[1] == 'all':
if sys.argv[2] == 'p':
	plt.title('Parallel: Number of Faces Collapsed vs Time')
else:
	plt.title('Sequential: Number of Faces Collapsed vs Time')
plt.xlabel('Time (ms)')
plt.ylabel('Number of Faces Collapsed')

plt.show()
