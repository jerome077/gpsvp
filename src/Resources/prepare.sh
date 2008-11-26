#!/bin/bash

ls *.ico | awk 'BEGIN{print "(define (convert filename1 filename2) (let* ((image (car (gimp-file-load RUN-NONINTERACTIVE filename1 filename1))) (drawable (car (gimp-image-get-active-layer image)))) (gimp-file-save RUN-NONINTERACTIVE image drawable filename2 filename2) (gimp-image-delete image)))"}{sub(/.ico$/,""); print "(convert \""$1".ico\" \""$1".png\")"}' | gimp -i -b -
