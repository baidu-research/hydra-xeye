.global mbinImgSipp

!;#####################################################
.section ".data"
.align 4
mbinImgSipp:
 .incbin "output/svuSippImg.shvdlib"

!;#####################################################
.section ".shv0.S.init"
cmxInits1:
 .fill (131072), 1, 0xCD
.section ".shv1.S.init"
cmxInits2:
 .fill (131072), 1, 0xCD
.section ".shv2.S.init"
cmxInits3:
 .fill (131072), 1, 0xCD
