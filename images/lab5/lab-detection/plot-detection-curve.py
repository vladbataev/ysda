#!/usr/bin/python2
# coding: utf8

from matplotlib import use
use('AGG')

from argparse import ArgumentParser
from matplotlib.pyplot import figure, subplot, plot, legend, savefig, close
from matplotlib.font_manager import FontProperties
from os.path import isfile, exists
from os import access, R_OK

parser = ArgumentParser(description = 'Draw ROC for roc-points files')
add_arg = parser.add_argument
add_arg('--output', type = str, required = True, help = 'output png file path')
add_arg('--roc-files', nargs = '+', required = True, help = 'roc files to plot')
add_arg('--titles', nargs = '+', required = True, help = 'titles for every graphic')
add_arg('--data-points', action = 'store_true', help = 'draw points from file')
add_arg('--minor-grid', action = 'store_true', help = 'draw detailed grid')

arg = parser.parse_args()
if exists(arg.output):
    print 'output file "%s" already exists' % arg.output
    exit(1)

roc_files = arg.roc_files
titles = arg.titles
line_style = '.-' if arg.data_points else '-'

if len(titles) != len(roc_files):
    print 'roc files count must match titles count'
    exit(1)

data = []
for roc_file, title in zip(roc_files, titles):
    if not isfile(roc_file) or not access(roc_file, R_OK):
        print 'inaccessible roc file: %s' % roc_file
        continue
    lines = open(roc_file).readlines()
    lines = filter(lambda x: x != '', map(lambda s: s.split('#')[0].strip(), lines))
    x_vals, y_vals = zip(*map(lambda s: map(float, s.split()[0:2]), lines))
    data.append((x_vals, y_vals, title.replace('\\n', '\n')))
del lines

# setting good font (preferably, narrow)
font = FontProperties(family = 'PT Sans Narrow', size = 14)
figure(figsize = (20, 20), dpi = 101)
axes = subplot()
axes.set_xlim([-1, 101])
axes.set_ylim([-1, 101])
axes.set_xlabel('Recall, %', fontproperties = font)
axes.set_ylabel('Precision, %', fontproperties = font)
axes.set_aspect('equal')
axes.set_xticks(xrange(0, 101, 5), minor = False)
axes.set_xticks(xrange(0, 101, 1), minor = True)
axes.set_yticks(xrange(0, 101, 5), minor = False)
axes.set_yticks(xrange(0, 101, 1), minor = True)
axes.grid(which = 'major', alpha = 0.9)
if arg.minor_grid:
    axes.grid(which = 'minor', alpha = 0.2)

for values in data:
    plot(values[0], values[1], line_style, aa = True, alpha = 0.7, label = values[2])
map(lambda l: l.set_fontproperties(font), axes.get_xticklabels() + axes.get_yticklabels())

legend(loc = 'lower left', prop = font)
savefig(arg.output, bbox_inches = 'tight')
close()
