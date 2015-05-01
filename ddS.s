	.verstamp	7 20
	.option	pic2
	.text	
	.align	2
	.file	2 "ddS.c"
	.globl	decodeScan
	.loc	2 1
 #   1	void* decodeScan(int w , int h, void* inP  ,void *outP , void* desP) {
	.ent	decodeScan 2
decodeScan:
	.option	O1
	.set	 noreorder
	.cpload	$25
	.set	 reorder
	sw	$4, 0($sp)
	sw	$5, 4($sp)
	sw	$6, 8($sp)
	sw	$7, 12($sp)
	.frame	$sp, 0, $31
	.loc	2 1
	.loc	2 2
 #   2	  return desP;
	lw	$2, 16($sp)
	.livereg	0x2000FF0E,0x00000FFF
	j	$31
	.end	decodeScan
