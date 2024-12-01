
/home/julia/projects/RV32I-Interpreter/rv32i_code_examples/test:     формат файла elf32-littleriscv


Дизассемблирование раздела .text:

00010074 <main>:
   10074:	00500d13          	addi	x26,x0,5
   10078:	00000d93          	addi	x27,x0,0
   1007c:	001d8d93          	addi	x27,x27,1
   10080:	fe0e8e13          	addi	x28,x29,-32
   10084:	fe0e8e13          	addi	x28,x29,-32
   10088:	fe0e8e13          	addi	x28,x29,-32
   1008c:	fe0e8e13          	addi	x28,x29,-32
   10090:	fe0e8e13          	addi	x28,x29,-32
   10094:	fe0e8e13          	addi	x28,x29,-32
   10098:	fe0e8e13          	addi	x28,x29,-32
   1009c:	fe0e8e13          	addi	x28,x29,-32
   100a0:	fe0e8e13          	addi	x28,x29,-32
   100a4:	fe0e8e13          	addi	x28,x29,-32
   100a8:	fe0e8e13          	addi	x28,x29,-32
   100ac:	fe0e8e13          	addi	x28,x29,-32
   100b0:	fe0e8e13          	addi	x28,x29,-32
   100b4:	fe0e8e13          	addi	x28,x29,-32
   100b8:	fe0e8e13          	addi	x28,x29,-32
   100bc:	fe0e8e13          	addi	x28,x29,-32
   100c0:	fe0e8e13          	addi	x28,x29,-32
   100c4:	fbad9ce3          	bne	x27,x26,1007c <main+0x8>
   100c8:	00100073          	ebreak

Дизассемблирование раздела .riscv.attributes:

00000000 <.riscv.attributes>:
   0:	4b41                	.insn	2, 0x4b41
   2:	0000                	.insn	2, 0x
   4:	7200                	.insn	2, 0x7200
   6:	7369                	.insn	2, 0x7369
   8:	01007663          	bgeu	x0,x16,14 <main-0x10060>
   c:	0041                	.insn	2, 0x0041
   e:	0000                	.insn	2, 0x
  10:	7205                	.insn	2, 0x7205
  12:	3376                	.insn	2, 0x3376
  14:	6932                	.insn	2, 0x6932
  16:	7032                	.insn	2, 0x7032
  18:	5f31                	.insn	2, 0x5f31
  1a:	326d                	.insn	2, 0x326d
  1c:	3070                	.insn	2, 0x3070
  1e:	615f 7032 5f31      	.insn	6, 0x5f317032615f
  24:	3266                	.insn	2, 0x3266
  26:	3270                	.insn	2, 0x3270
  28:	645f 7032 5f32      	.insn	6, 0x5f327032645f
  2e:	697a                	.insn	2, 0x697a
  30:	32727363          	bgeu	x4,x7,356 <main-0xfd1e>
  34:	3070                	.insn	2, 0x3070
  36:	7a5f 6669 6e65      	.insn	6, 0x6e6566697a5f
  3c:	32696563          	bltu	x18,x6,366 <main-0xfd0e>
  40:	3070                	.insn	2, 0x3070
  42:	7a5f 6d6d 6c75      	.insn	6, 0x6c756d6d7a5f
  48:	7031                	.insn	2, 0x7031
  4a:	0030                	.insn	2, 0x0030
