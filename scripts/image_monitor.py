import os
import argparse

import numpy as np
import cv2
import matplotlib
matplotlib.use('TkAgg')

import matplotlib.pyplot as plt
from matplotlib import animation

def animate(filename, ax, tm_method):
    _, ext = os.path.splitext(filename)
    img = cv2.imread(filename, cv2.IMREAD_UNCHANGED)

    if ext == '.hdr':
        if tm_method == 'reinhard':
            tmo = cv2.createTonemapReinhard()
            img = tmo.process(img)
        elif tm_method == 'durand':
            tmo = cv2.createTonemapDurand()
            img = tmo.process(img)
        else:
            maxval = 1.0
            img = np.power(img / maxval, 1.0 / 2.2)

    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    img = np.clip(img, 0.0, 1.0)

    ax.set_data(img)

def main():
    parser = argparse.ArgumentParser(description='Image monitor')
    parser.add_argument('-i', '--input', type=str, required=True,
                        help='Image that you want to monitor (.jpg, .png, .hdr)')
    parser.add_argument('-t', '--tonemap', default='reinhard',
                        help='Tonemapper used for HDR image (defalt: reinhard02)')

    args = parser.parse_args()

    # Attemp to load image
    img = cv2.imread(args.input, cv2.IMREAD_UNCHANGED)
    if img is None:
        raise Exception('Image file "{:s}" does not exist!'.format(args.input))

    height, width, dims = img.shape

    fig = plt.figure()
    plt.axis('off')
    ax = plt.imshow(np.zeros((height, width, dims)))

    init_fn = lambda : animate(args.input, ax, args.tonemap)
    anim_fn = lambda i: animate(args.input, ax, args.tonemap)
    anim = animation.FuncAnimation(fig, anim_fn, init_func=init_fn, interval=1000)
    plt.show()

if __name__ == '__main__':
    main()
