#!/usr/bin/python2
# coding: utf8

from argparse import ArgumentParser, RawTextHelpFormatter
from os.path import isdir, isfile, join, exists, abspath, basename, splitext
from os import access, R_OK, W_OK, X_OK, makedirs
from glob import iglob

desc_epilog = """\
This script is designed to GET raw detection list with confidences
in addition to ground truth data from image base that was used to
get detection list and PRODUCE roc curve precision-recall based on
confidence thresholding. The result will be roc.txt file with points
of roc curve.

ATTENTION!
Read next paragraph carefully.
Any detection that has good intersecion over union ratio is considered
True Positive. If a ground truth bounding box matches by several
detection bboxes with accepted IoU -- all of that detections will be
ignored expect one that gives best confidence. Overall count of ignored
multiple detections will be printed out to stdout. If there are any
ignore-marked bounding boxes in ground-truth data then any detection
with accepted IoU against these bboxes will be ignored and info
about ignore-marked samples hits will be printed to stdout too.

INPUT-BASE must be a path to directory with image base in standart
format: "imgs" subdir with images named like SAMPLE_NAME.EXT and "gt"
subdir with bounding-rectangles in the format described below with
file names like SAMPLE_NAME.txt.

DETECTIONS-LIST must be a path to a file that contains examining
detections. File may contain comments started with # symbol. Empty
or started with # symbol lines are allowed. Detection format is one
bounding-box per line like: "SAMPLE_NAME; X; Y; W; H; CONFIDENCE".

WORKDIR must be a path to directory. The directory may exist or not.
If directory doesn't exist then it will be created. Output file
"roc.txt" will be placed directly into workdir directory.

FLUSH-BARRIER is a float value that configures policy for graph points
production. On thresholding process too many points can be produced so
policy is: after some point is fixed next one will be produced only
if it outstands the previous on one of axes by value which is bigger
than flush-barrier. Axes use percents scale from 0 to 100 thus flush-
barrier does the same. Note that it does not guarantee limitation on
distance between sequent points because this script is just responding
to next point distance but is unable to control it. But you will see
warning if gap, which is bigger than twice of flush-barrier, will be
accepted.

GOOD-IOU is a percentage value that separates accepted and declined bboxes.
For each detection a ground-truth bounding box is selected so maximum
intersection over union percentage is reached. If the maximum is lower
than good-iou level then detection is considered as false positive.

ALSO: ground-truth format is: one bounding-box per line, each line
contains space-symbols separated list of next fields:
01. class label, string with no spaces
02. bounding box X
03. bounding box Y
04. bounding box W
05. bounding box H
06. occlusion flag, 0/1
07. visible bounding box X
08. visible bounding box Y
09. visible bounding box W
10. visible bounding box H
11. ignore flag, 0/1, ignore means no usage of the bbox in positive set
12. bounding box angle, 0-360

Last two may be omitted. If so its values will be defaulted to zero.
"""

parser = ArgumentParser(description = 'Evaluates detections against ground truth data', \
                        epilog = desc_epilog, formatter_class = RawTextHelpFormatter)
add_arg = parser.add_argument
add_arg('--input-base', type = str, required = True, \
        help = 'path to directory with sample images and gt')
add_arg('--detections-list', type = str, required = True, \
        help = 'path to detections list in format "name;x;y;w;h;confidence"')
add_arg('--workdir', type = str, required = True, \
        help = 'path to directory where work files and output results are stored')
add_arg('--flush-barrier', type = float, required = True, \
        help = 'maximum allowed gap between sequent points on graphs')
add_arg('--good-iou', type = float, required = True, \
        help = 'minimum accepted percentage of intersection-over-union against gt')

arg = parser.parse_args()

# sorting arguments and checking arguments
is_accessable_dir = lambda name: isdir(name) and access(name, R_OK | X_OK)
is_readable_file = lambda name: isfile(name) and access(name, R_OK)

datebase_dir = abspath(arg.input_base)
gt_dir = join(datebase_dir, 'gt')
img_dir = join(datebase_dir, 'imgs')
if not all(map(is_accessable_dir, [datebase_dir, gt_dir, img_dir])):
    print datebase_dir, gt_dir, img_dir
    print 'input base directories accessing failure'
    exit(1)

workdir = abspath(arg.workdir)
if not exists(workdir):
    makedirs(workdir)

barrier = arg.flush_barrier
if barrier < 0. or barrier > 100.:
    print 'given flush barrier value is meaningless'
    exit(1)

iou_threshold = arg.good_iou
if iou_threshold < 0. or iou_threshold > 100.:
    print 'given good-iou value is meaningless'
    exit(1)
if iou_threshold < 1.:
    print '# WARNING: good-iou takes percentage value'

detections_file = abspath(arg.detections_list)
if not is_readable_file(detections_file):
    print 'detections list file accessing failure'
    exit(1)

# helper functions to calculate intersection-over-union
def overlap(l, r):
    # format: l = [point, length], same for r
    # returns overlap for 1d segments
    if l[0] > r[0]:
        l, r = r, l
    far_r, far_l = map(sum, (r, l))
    if r[0] > far_l:
        return 0
    if far_l > far_r:
        return r[1]
    return far_l - r[0]

def IoU(a, b):
    # format: a = [x, y, w, h], same for b
    # returns overlap percentage: IoverU
    x_overlap = overlap((a[0], a[2]), (b[0], b[2]))
    y_overlap = overlap((a[1], a[3]), (b[1], b[3]))
    common = 1. * x_overlap * y_overlap
    union = a[2] * a[3] + b[2] * b[3] - common
    return 100. * common / union

# loading ground truth data
samplename = lambda s: splitext(basename(s))[0]
#print '# reading ground truth files'
# GT is the map from sample name to list of bboxes: [[x y w h ignore best-confidence]],
# where best-confidence is:
#   None if gt bbox with number INDEX isn't detected OR
#   float value if that gt bbox detected with conf X and it's max confidence across multiple dets.
gt = dict()

for image_path in iglob(join(img_dir, '*')):
    # ignoring hidden files
    if image_path[0] == '.':
        continue
    sample = samplename(image_path)
    gt_file = join(gt_dir, sample + '.txt')
    if not exists(gt_file):
        #print 'gt file is missing for sample "%s", assumming no objects' % sample
        gt[sample] = []
        continue
    gt_bboxes = []
    for gt_line in open(gt_file).readlines():
        fields = gt_line.split()
        if len(fields) == 10:
            fields += ['0', '0']
        if len(fields) != 12:
            print 'bad gt format for sample "%s"' % sample
            exit(1)
        ignored = fields[10] == '1'
        gt_bboxes.append(map(int, fields[1:5]) + [ignored, None])
    gt[sample] = gt_bboxes

# list of changes: change means change of tp/fp/fn when confidence threshold becomes lower
# then confidence value of detection which is bound to change-point:
#       O--------|--------------|----------------->> increasing confidence
#           (accepted)     (declined)

# When moving through point (upper -> lower values of confidence) in [changes] list:
#   positive point (accepted bbox):  -1 FN   +1 TP   +0 FP  [ +1 TP over confidence threshold]
#   negative point (declined bbox):  +0 FN   +0 TP   +1 FP  [ +1 FP over confidence threshold]
# Precision and recall can be calculated via:
#   precision = tp / (tp + fp)
#   recall = tp / (tp + fn) = tp / gt_count

# So we need know changes of (tp + fp) and (tp):
#   positive point (accepted bbox):  +1 TP   +1 (TP + FP)
#   negative point (declined bbox):  +0 TP   +1 (TP + FP)
# Thus: every point means (TP + FP) increases by 1, (TP) behaviour may differ.
# Conclusion: we need to save diffs of TP value.
# [changes] -- list of tuples: (confidence, TP change):
#   on accepted: (confidence, +1)
#   on declined: (confidence, +0)
changes = []
multiple_detections = 0

#print '# reading detections file'
for detection_line in open(detections_file).readlines():
    fields = detection_line.split(';')
    if len(fields) != 6:
        print 'detection-file format is violated'
        exit(1)
    sample = samplename(fields[0])
    confidence = float(fields[5])
    bbox = map(int, fields[1:5])
    gt_list = gt.get(sample, None)
    if gt_list == None:
        print 'unknown sample "%s" in detection file' % sample
        exit(1)
    iou_vals = map(lambda gt_elem: (IoU(gt_elem[0:4], bbox), gt_elem), gt_list)
    if iou_vals == []:
        changes.append((confidence, 0))
        continue
    iou_value, best_gt = max(iou_vals) # by default max on first element in tuple
    if iou_value < iou_threshold:
        # inconditional false positive
        changes.append((confidence, 0))
    else:
        # need to select best match for this gt bbox
        if best_gt[5]:
            # have multiple match and previous detection
            multiple_detections += 1
            best_gt[5] = max(best_gt[5], confidence)
        else:
            # no multiple detections so far for this gt bbox
            best_gt[5] = confidence

print '#    caught %d false positives' % len(changes)
#print '# processing ground truth match-lists'
undetected_count = 0
good_gt_count = 0
ignored_count = 0
ignored_found = 0

for gt_list in gt.itervalues():
    for gt_elem in gt_list:
        ignore, confidence = gt_elem[4:6]
        if not ignore:
            good_gt_count += 1
            if not confidence:
                undetected_count += 1
            else:
                changes.append((confidence, 1))
        else:
            ignored_count += 1
            ignored_found += int(confidence != None)

print '#    undetected objects count: %d of %d good gt' % (undetected_count, good_gt_count)
#print '#    ignore-marked objects: %d detected out of %d' % (ignored_found, ignored_count)
#print '# sorting confidence list'
changes.sort() # by default using first element as key, ascending

#print '# confidence thresholding pass'
# starting from threshold = above max confidence of detection:
# so there are no single one detection accepted, so FP = TP = 0, FN = all gt count
tp_value = 0
tpfp_value = 0 # tp + fp

precision = lambda: (100. * tp_value / tpfp_value) if tpfp_value != 0 else None
recall = lambda: 100. * tp_value / good_gt_count

# data of previous point to measure gap between sequent points
prev_precision = precision()
prev_recall = recall()

roc_file = open(join(workdir, 'roc.txt'), 'w')
produce = roc_file.write
produce('# RECALL | PRECISION | CONFIDENCE\n')

auc_score = 0
force_point = True # force first point to be written without gap checks
current_idx = len(changes) - 1
while current_idx >= 0:
    confidence, tp_change = changes[current_idx]
    # changes in this confidence point
    tp_value += tp_change
    tpfp_value += 1
    # recalculating precision/recall
    curr_precision = precision()
    curr_recall = recall()
    # calculating points gap
    precision_diff = abs(prev_precision - curr_precision) if prev_precision else 1. - curr_precision
    recall_diff = curr_recall - prev_recall
    # checking gap (or last point)
    gap = max(precision_diff, recall_diff)
    if gap > 2 * barrier:
        msg = '#    big gap %0.3f occured (barrier %0.3f), AUC might be affected' % (gap, barrier)
        #print msg
    if gap > barrier or current_idx == 0 or force_point:
        produce("%0.3f %0.3f %0.3f\n" % (curr_recall, curr_precision, confidence))
        if not force_point:
            auc_score += (0.5 * recall_diff * (prev_precision + curr_precision))
        else:
            force_point = False
        prev_precision = curr_precision
        prev_recall = curr_recall
    current_idx -= 1

auc_msg = '# AUC score is %.2f\n' % (auc_score / 100)
produce(auc_msg)
roc_file.close()
print auc_msg
