; https://superuser.com/questions/77429/using-gimp-to-batch-convert-images-to-another-format-in-windows/1558320#1558320

(define (xmc_to_png in_filename
                    out_filename
                    )
    (let* (
            (image (car (gimp-file-load RUN-NONINTERACTIVE in_filename in_filename)))
            (drawable (car (gimp-image-get-active-layer image)))
        )
        (gimp-file-save RUN-NONINTERACTIVE image drawable out_filename out_filename)
        ; nope, thanks (gimp-image-delete image)
    )
)
