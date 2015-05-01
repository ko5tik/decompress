   # register definitions
#define scanWidth $4
#define scanHeight $5
#define inPtr $6
#define outPtr $7
#define descPtr $8

 #interesting constants
#define zero $0
	# we can misuse $2 for iinternal puproses , before we return value :	)
#define ones $2

	#predictor values
#define pred $9
	# predictor for the first pixel in row
#define predFirst $20
	# how many rows and columns to do
#define row2do $5
#define col2do $10

	# huffman codes storage
#define huffBits $11
#define huffBitsFree $12

	# working data to load / store
#define work $13

	# here we get descriptors loaded
#define bitsCode $14
#define bitsSym  $15
	# termination condition
#define terminate $16

	# load place
#define loadB $17
	#temorary value
#define tempVal $18
	# 32
#define thirtyTwo $19

#define scanCheck $21	
		
	.verstamp	7 20
	.option	pic2
	.text	
	.align	2
	.globl	decodeScan

	.ent	decodeScan 2
decodeScan:
	.option	O1
	.set	 noreorder
	.cpload	$25
	.set	 reorder

	.frame	$sp, 0, $31

	#get dscriptors into descPtr
	lw	descPtr , 16($sp)
	#init ones
	not ones , zero	
	# init very first predictor value
	li pred , 0x80	
	#init huff bits
	li huffBits , 0
	# and  bits free too
	li huffBitsFree , 32
	move col2do , scanWidth
	li thirtyTwo , 32
	sub scanCheck , scanWidth , 1
	
	# load compressed values
loadJpeg:
	# do if we have at least 16 bits free
	blt huffBitsFree , 16 , decodeDiff
	lbu loadB , (inPtr)
	sub huffBitsFree , 8
	sll work , loadB , huffBitsFree
	or huffBits , huffBits , work
	bne loadB , 0xff , afterStuff1
	lbu loadB , 1(inPtr)
	add inPtr , 1
	# check if it's 0
	bnez loadB , terminateDecompress

afterStuff1:	
	# second byte ....
	lbu loadB , 1(inPtr)
	sub huffBitsFree , 8
	sll work , loadB , huffBitsFree
	or huffBits , huffBits , work
	bne loadB , 0xff , afterStuff2		

	lbu loadB , 2(inPtr)
	add inPtr , 1
	# check if it's 0
	bnez loadB , terminateDecompress
	# it's not 0 , also , terminate decompression on jpeg marker

afterStuff2:	
	# advance pointer ...
	add inPtr , 2
	
decodeDiff:	
	# get upper 8 bit ...
	srl work , huffBits , 24
	# shift it around ...
	sll work , work , 3
	# and produce descriptor adress
 	add work , work , descPtr	
	#adjust bits free
	add huffBitsFree , huffBitsFree , bitsCode
	add huffBitsFree , huffBitsFree , bitsSym
	ld bitsCode ,(work)
	
	
	# eliminate code
	sll huffBits , huffBits , bitsCode

	# check if zero bit difference ...
	bnel bitsSym ,zero , afterZero
	# expand zero difference ( nullify it )
	sb pred , (outPtr)
	add outPtr , 1
	# leave predictor untouched ...
	sub col2do , 1	
	b afterPredictor
afterZero:	
	# ok , difference value needs to be expanded
		
	# compute right shift
	sub work , thirtyTwo , bitsSym
	# get  coded difference
	sra work , huffBits , work
	#eliminate difference bits
	sll huffBits , huffBits , bitsSym
	# expand not zero difference
	sll tempVal , ones , bitsSym
	xor work , work , tempVal
	# take care for negative difference ,
	## add one if this
	bgez work , afterDecode
	add work , 1

afterDecode:
	# here we have difference decoded
	# compute pixel value
	add work , work , pred
	# and save it
	sb work , (outPtr)
	add outPtr , 1
	# init predictor now for performance sake
	sub col2do , 1	
	move pred , work
afterPredictor:	
	
	# here we must advance coordinates and recompute predictor
	bne col2do , scanCheck , afterFirstPixel
	# for the first pixel
	move predFirst , pred
	b loadJpeg
afterFirstPixel:		
	bnez col2do , loadJpeg
	# init next row
	sub row2do , 1
	move pred , predFirst
	move col2do , scanWidth
	bnez row2do , loadJpeg
	b end_scan
	
terminateDecompress:			
	#finish decompression work , do not load anymore


	# get upper 8 bit ...
	srl work , huffBits , 24
	# shift it around ...
	sll work , work , 3
	# and produce descriptor adress
 	add work , work , descPtr
	ld bitsCode ,(work)
	
	#adjust bits free
	add huffBitsFree , huffBitsFree , bitsCode
	add huffBitsFree , huffBitsFree , bitsSym
	
	# eliminate code
	sll huffBits , huffBits , bitsCode

	# check if zero bit difference ...
	bnez bitsSym , afterZero2
	# expand zero difference ( nullify it )
	sb zero , (outPtr)
	add outPtr , 1
	# leave predictor untouched ...
	b afterPredictor2
afterZero2:	
	# ok , difference value needs to be expanded	
	# compute right shift
	sub work , thirtyTwo , bitsSym
	# get  coded difference
	sra work , huffBits , work
	#eliminate difference bits
	sll huffBits , huffBits , bitsSym
	# expand not zero difference
	sll tempVal , ones , bitsSym
	xor work , work , tempVal
	# take care for negative difference ,
	## add one if this
	bgez work , afterDecode2
	add work , 1

afterDecode2:
	# here we have difference decoded
	# compute pixel value
	add work , work , pred
	# and save it
	sb work , (outPtr)
	add outPtr , 1
	# init predictor now for performance sake
	move pred , work

afterPredictor2:	
	blt huffBitsFree , 24 , decodeDiff
	
	
	
		
 #  return status value - everythig is OK
end_scan:

	move $2 , outPtr
	
	.livereg	0x2000FF0E,0x00000FFF
	j	$31
	.end	decodeScan
