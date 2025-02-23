
thread_incr_mod:     file format elf64-littleaarch64


Disassembly of section .init:

0000000000001618 <_init>:
    1618:	d503201f 	nop
    161c:	a9bf7bfd 	stp	x29, x30, [sp, #-16]!
    1620:	910003fd 	mov	x29, sp
    1624:	94000074 	bl	17f4 <call_weak_fn>
    1628:	a8c17bfd 	ldp	x29, x30, [sp], #16
    162c:	d65f03c0 	ret

Disassembly of section .plt:

0000000000001630 <.plt>:
    1630:	a9bf7bf0 	stp	x16, x30, [sp, #-16]!
    1634:	d00000f0 	adrp	x16, 1f000 <__FRAME_END__+0x1c0e0>
    1638:	f947fe11 	ldr	x17, [x16, #4088]
    163c:	913fe210 	add	x16, x16, #0xff8
    1640:	d61f0220 	br	x17
    1644:	d503201f 	nop
    1648:	d503201f 	nop
    164c:	d503201f 	nop

0000000000001650 <_exit@plt>:
    1650:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    1654:	f9400211 	ldr	x17, [x16]
    1658:	91000210 	add	x16, x16, #0x0
    165c:	d61f0220 	br	x17

0000000000001660 <fputs@plt>:
    1660:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    1664:	f9400611 	ldr	x17, [x16, #8]
    1668:	91002210 	add	x16, x16, #0x8
    166c:	d61f0220 	br	x17

0000000000001670 <exit@plt>:
    1670:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    1674:	f9400a11 	ldr	x17, [x16, #16]
    1678:	91004210 	add	x16, x16, #0x10
    167c:	d61f0220 	br	x17

0000000000001680 <__libc_start_main@plt>:
    1680:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    1684:	f9400e11 	ldr	x17, [x16, #24]
    1688:	91006210 	add	x16, x16, #0x18
    168c:	d61f0220 	br	x17

0000000000001690 <__cxa_finalize@plt>:
    1690:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    1694:	f9401211 	ldr	x17, [x16, #32]
    1698:	91008210 	add	x16, x16, #0x20
    169c:	d61f0220 	br	x17

00000000000016a0 <snprintf@plt>:
    16a0:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    16a4:	f9401611 	ldr	x17, [x16, #40]
    16a8:	9100a210 	add	x16, x16, #0x28
    16ac:	d61f0220 	br	x17

00000000000016b0 <strerror@plt>:
    16b0:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    16b4:	f9401a11 	ldr	x17, [x16, #48]
    16b8:	9100c210 	add	x16, x16, #0x30
    16bc:	d61f0220 	br	x17

00000000000016c0 <__gmon_start__@plt>:
    16c0:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    16c4:	f9401e11 	ldr	x17, [x16, #56]
    16c8:	9100e210 	add	x16, x16, #0x38
    16cc:	d61f0220 	br	x17

00000000000016d0 <abort@plt>:
    16d0:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    16d4:	f9402211 	ldr	x17, [x16, #64]
    16d8:	91010210 	add	x16, x16, #0x40
    16dc:	d61f0220 	br	x17

00000000000016e0 <strtol@plt>:
    16e0:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    16e4:	f9402611 	ldr	x17, [x16, #72]
    16e8:	91012210 	add	x16, x16, #0x48
    16ec:	d61f0220 	br	x17

00000000000016f0 <fwrite@plt>:
    16f0:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    16f4:	f9402a11 	ldr	x17, [x16, #80]
    16f8:	91014210 	add	x16, x16, #0x50
    16fc:	d61f0220 	br	x17

0000000000001700 <pthread_create@plt>:
    1700:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    1704:	f9402e11 	ldr	x17, [x16, #88]
    1708:	91016210 	add	x16, x16, #0x58
    170c:	d61f0220 	br	x17

0000000000001710 <fflush@plt>:
    1710:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    1714:	f9403211 	ldr	x17, [x16, #96]
    1718:	91018210 	add	x16, x16, #0x60
    171c:	d61f0220 	br	x17

0000000000001720 <vsnprintf@plt>:
    1720:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    1724:	f9403611 	ldr	x17, [x16, #104]
    1728:	9101a210 	add	x16, x16, #0x68
    172c:	d61f0220 	br	x17

0000000000001730 <vfprintf@plt>:
    1730:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    1734:	f9403a11 	ldr	x17, [x16, #112]
    1738:	9101c210 	add	x16, x16, #0x70
    173c:	d61f0220 	br	x17

0000000000001740 <printf@plt>:
    1740:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    1744:	f9403e11 	ldr	x17, [x16, #120]
    1748:	9101e210 	add	x16, x16, #0x78
    174c:	d61f0220 	br	x17

0000000000001750 <__errno_location@plt>:
    1750:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    1754:	f9404211 	ldr	x17, [x16, #128]
    1758:	91020210 	add	x16, x16, #0x80
    175c:	d61f0220 	br	x17

0000000000001760 <pthread_join@plt>:
    1760:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    1764:	f9404611 	ldr	x17, [x16, #136]
    1768:	91022210 	add	x16, x16, #0x88
    176c:	d61f0220 	br	x17

0000000000001770 <getenv@plt>:
    1770:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    1774:	f9404a11 	ldr	x17, [x16, #144]
    1778:	91024210 	add	x16, x16, #0x90
    177c:	d61f0220 	br	x17

0000000000001780 <fprintf@plt>:
    1780:	f00000f0 	adrp	x16, 20000 <_exit@GLIBC_2.17>
    1784:	f9404e11 	ldr	x17, [x16, #152]
    1788:	91026210 	add	x16, x16, #0x98
    178c:	d61f0220 	br	x17

Disassembly of section .text:

00000000000017c0 <_start>:
    17c0:	d503201f 	nop
    17c4:	d280001d 	mov	x29, #0x0                   	// #0
    17c8:	d280001e 	mov	x30, #0x0                   	// #0
    17cc:	aa0003e5 	mov	x5, x0
    17d0:	f94003e1 	ldr	x1, [sp]
    17d4:	910023e2 	add	x2, sp, #0x8
    17d8:	910003e6 	mov	x6, sp
    17dc:	d00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    17e0:	f947ec00 	ldr	x0, [x0, #4056]
    17e4:	d2800003 	mov	x3, #0x0                   	// #0
    17e8:	d2800004 	mov	x4, #0x0                   	// #0
    17ec:	97ffffa5 	bl	1680 <__libc_start_main@plt>
    17f0:	97ffffb8 	bl	16d0 <abort@plt>

00000000000017f4 <call_weak_fn>:
    17f4:	d00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    17f8:	f947e800 	ldr	x0, [x0, #4048]
    17fc:	b4000040 	cbz	x0, 1804 <call_weak_fn+0x10>
    1800:	17ffffb0 	b	16c0 <__gmon_start__@plt>
    1804:	d65f03c0 	ret
    1808:	d503201f 	nop
    180c:	d503201f 	nop

0000000000001810 <deregister_tm_clones>:
    1810:	f00000e0 	adrp	x0, 20000 <_exit@GLIBC_2.17>
    1814:	91138000 	add	x0, x0, #0x4e0
    1818:	f00000e1 	adrp	x1, 20000 <_exit@GLIBC_2.17>
    181c:	91138021 	add	x1, x1, #0x4e0
    1820:	eb00003f 	cmp	x1, x0
    1824:	540000c0 	b.eq	183c <deregister_tm_clones+0x2c>  // b.none
    1828:	d00000e1 	adrp	x1, 1f000 <__FRAME_END__+0x1c0e0>
    182c:	f947d821 	ldr	x1, [x1, #4016]
    1830:	b4000061 	cbz	x1, 183c <deregister_tm_clones+0x2c>
    1834:	aa0103f0 	mov	x16, x1
    1838:	d61f0200 	br	x16
    183c:	d65f03c0 	ret

0000000000001840 <register_tm_clones>:
    1840:	f00000e0 	adrp	x0, 20000 <_exit@GLIBC_2.17>
    1844:	91138000 	add	x0, x0, #0x4e0
    1848:	f00000e1 	adrp	x1, 20000 <_exit@GLIBC_2.17>
    184c:	91138021 	add	x1, x1, #0x4e0
    1850:	cb000021 	sub	x1, x1, x0
    1854:	d37ffc22 	lsr	x2, x1, #63
    1858:	8b810c41 	add	x1, x2, x1, asr #3
    185c:	9341fc21 	asr	x1, x1, #1
    1860:	b40000c1 	cbz	x1, 1878 <register_tm_clones+0x38>
    1864:	d00000e2 	adrp	x2, 1f000 <__FRAME_END__+0x1c0e0>
    1868:	f947f042 	ldr	x2, [x2, #4064]
    186c:	b4000062 	cbz	x2, 1878 <register_tm_clones+0x38>
    1870:	aa0203f0 	mov	x16, x2
    1874:	d61f0200 	br	x16
    1878:	d65f03c0 	ret
    187c:	d503201f 	nop

0000000000001880 <__do_global_dtors_aux>:
    1880:	a9be7bfd 	stp	x29, x30, [sp, #-32]!
    1884:	910003fd 	mov	x29, sp
    1888:	f9000bf3 	str	x19, [sp, #16]
    188c:	f00000f3 	adrp	x19, 20000 <_exit@GLIBC_2.17>
    1890:	39538260 	ldrb	w0, [x19, #1248]
    1894:	35000140 	cbnz	w0, 18bc <__do_global_dtors_aux+0x3c>
    1898:	d00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    189c:	f947dc00 	ldr	x0, [x0, #4024]
    18a0:	b4000080 	cbz	x0, 18b0 <__do_global_dtors_aux+0x30>
    18a4:	f00000e0 	adrp	x0, 20000 <_exit@GLIBC_2.17>
    18a8:	f9405400 	ldr	x0, [x0, #168]
    18ac:	97ffff79 	bl	1690 <__cxa_finalize@plt>
    18b0:	97ffffd8 	bl	1810 <deregister_tm_clones>
    18b4:	52800020 	mov	w0, #0x1                   	// #1
    18b8:	39138260 	strb	w0, [x19, #1248]
    18bc:	f9400bf3 	ldr	x19, [sp, #16]
    18c0:	a8c27bfd 	ldp	x29, x30, [sp], #32
    18c4:	d65f03c0 	ret
    18c8:	d503201f 	nop
    18cc:	d503201f 	nop

00000000000018d0 <frame_dummy>:
    18d0:	17ffffdc 	b	1840 <register_tm_clones>

00000000000018d4 <threadFunc>:

static volatile int glob = 0;   /* "volatile" prevents compiler optimizations
                                   of arithmetic operations on 'glob' */
static void *                   /* Loop 'arg' times incrementing 'glob' */
threadFunc(void *arg)
{
    18d4:	a9bc7bfd 	stp	x29, x30, [sp, #-64]!
    18d8:	910003fd 	mov	x29, sp
    18dc:	f9000fe0 	str	x0, [sp, #24]
    struct thread_args targs = *((struct thread_args *) arg);
    18e0:	f9400fe0 	ldr	x0, [sp, #24]
    18e4:	f9400000 	ldr	x0, [x0]
    18e8:	f90017e0 	str	x0, [sp, #40]
    int id = targs.id;
    18ec:	b9402be0 	ldr	w0, [sp, #40]
    18f0:	b9003be0 	str	w0, [sp, #56]
    int loops = targs.loops;
    18f4:	b9402fe0 	ldr	w0, [sp, #44]
    18f8:	b90037e0 	str	w0, [sp, #52]
    int loc, j;

    for (j = 0; j < loops; j++) {
    18fc:	b9003fff 	str	wzr, [sp, #60]
    1900:	14000017 	b	195c <threadFunc+0x88>
        loc = glob;
    1904:	f00000e0 	adrp	x0, 20000 <_exit@GLIBC_2.17>
    1908:	91139000 	add	x0, x0, #0x4e4
    190c:	b9400000 	ldr	w0, [x0]
    1910:	b90033e0 	str	w0, [sp, #48]
        loc++;
    1914:	b94033e0 	ldr	w0, [sp, #48]
    1918:	11000400 	add	w0, w0, #0x1
    191c:	b90033e0 	str	w0, [sp, #48]
        glob = loc;
    1920:	f00000e0 	adrp	x0, 20000 <_exit@GLIBC_2.17>
    1924:	91139000 	add	x0, x0, #0x4e4
    1928:	b94033e1 	ldr	w1, [sp, #48]
    192c:	b9000001 	str	w1, [x0]
        printf("[%d] glob = %d\n", id, glob);
    1930:	f00000e0 	adrp	x0, 20000 <_exit@GLIBC_2.17>
    1934:	91139000 	add	x0, x0, #0x4e4
    1938:	b9400000 	ldr	w0, [x0]
    193c:	2a0003e2 	mov	w2, w0
    1940:	b9403be1 	ldr	w1, [sp, #56]
    1944:	b0000000 	adrp	x0, 2000 <usageErr+0x10>
    1948:	91136000 	add	x0, x0, #0x4d8
    194c:	97ffff7d 	bl	1740 <printf@plt>
    for (j = 0; j < loops; j++) {
    1950:	b9403fe0 	ldr	w0, [sp, #60]
    1954:	11000400 	add	w0, w0, #0x1
    1958:	b9003fe0 	str	w0, [sp, #60]
    195c:	b9403fe1 	ldr	w1, [sp, #60]
    1960:	b94037e0 	ldr	w0, [sp, #52]
    1964:	6b00003f 	cmp	w1, w0
    1968:	54fffceb 	b.lt	1904 <threadFunc+0x30>  // b.tstop
    }

    return NULL;
    196c:	d2800000 	mov	x0, #0x0                   	// #0
}
    1970:	a8c47bfd 	ldp	x29, x30, [sp], #64
    1974:	d65f03c0 	ret

0000000000001978 <main>:

int
main(int argc, char *argv[])
{
    1978:	a9bb7bfd 	stp	x29, x30, [sp, #-80]!
    197c:	910003fd 	mov	x29, sp
    1980:	b9001fe0 	str	w0, [sp, #28]
    1984:	f9000be1 	str	x1, [sp, #16]
    pthread_t t1, t2;
    int loops, s;
    struct thread_args t1_args, t2_args;

    loops = (argc > 1) ? getInt(argv[1], GN_GT_0, "num-loops") : 10000000;
    1988:	b9401fe0 	ldr	w0, [sp, #28]
    198c:	7100041f 	cmp	w0, #0x1
    1990:	5400014d 	b.le	19b8 <main+0x40>
    1994:	f9400be0 	ldr	x0, [sp, #16]
    1998:	91002000 	add	x0, x0, #0x8
    199c:	f9400003 	ldr	x3, [x0]
    19a0:	b0000000 	adrp	x0, 2000 <usageErr+0x10>
    19a4:	9113a002 	add	x2, x0, #0x4e8
    19a8:	52800041 	mov	w1, #0x2                   	// #2
    19ac:	aa0303e0 	mov	x0, x3
    19b0:	940002a5 	bl	2444 <getInt>
    19b4:	14000003 	b	19c0 <main+0x48>
    19b8:	5292d000 	mov	w0, #0x9680                	// #38528
    19bc:	72a01300 	movk	w0, #0x98, lsl #16
    19c0:	b9004fe0 	str	w0, [sp, #76]
    t1_args.id = 1;
    19c4:	52800020 	mov	w0, #0x1                   	// #1
    19c8:	b90033e0 	str	w0, [sp, #48]
    t2_args.id = 2;
    19cc:	52800040 	mov	w0, #0x2                   	// #2
    19d0:	b9002be0 	str	w0, [sp, #40]
    t1_args.loops = loops;
    19d4:	b9404fe0 	ldr	w0, [sp, #76]
    19d8:	b90037e0 	str	w0, [sp, #52]
    t2_args.loops = loops;
    19dc:	b9404fe0 	ldr	w0, [sp, #76]
    19e0:	b9002fe0 	str	w0, [sp, #44]

    s = pthread_create(&t1, NULL, threadFunc, &t1_args);
    19e4:	9100c3e0 	add	x0, sp, #0x30
    19e8:	910103e4 	add	x4, sp, #0x40
    19ec:	aa0003e3 	mov	x3, x0
    19f0:	90000000 	adrp	x0, 1000 <__abi_tag+0xd88>
    19f4:	91235002 	add	x2, x0, #0x8d4
    19f8:	d2800001 	mov	x1, #0x0                   	// #0
    19fc:	aa0403e0 	mov	x0, x4
    1a00:	97ffff40 	bl	1700 <pthread_create@plt>
    1a04:	b9004be0 	str	w0, [sp, #72]
    if (s != 0)
    1a08:	b9404be0 	ldr	w0, [sp, #72]
    1a0c:	7100001f 	cmp	w0, #0x0
    1a10:	540000a0 	b.eq	1a24 <main+0xac>  // b.none
        errExitEN(s, "pthread_create");
    1a14:	b0000000 	adrp	x0, 2000 <usageErr+0x10>
    1a18:	9113e001 	add	x1, x0, #0x4f8
    1a1c:	b9404be0 	ldr	w0, [sp, #72]
    1a20:	94000122 	bl	1ea8 <errExitEN>
    s = pthread_create(&t2, NULL, threadFunc, &t2_args);
    1a24:	9100a3e0 	add	x0, sp, #0x28
    1a28:	9100e3e4 	add	x4, sp, #0x38
    1a2c:	aa0003e3 	mov	x3, x0
    1a30:	90000000 	adrp	x0, 1000 <__abi_tag+0xd88>
    1a34:	91235002 	add	x2, x0, #0x8d4
    1a38:	d2800001 	mov	x1, #0x0                   	// #0
    1a3c:	aa0403e0 	mov	x0, x4
    1a40:	97ffff30 	bl	1700 <pthread_create@plt>
    1a44:	b9004be0 	str	w0, [sp, #72]
    if (s != 0)
    1a48:	b9404be0 	ldr	w0, [sp, #72]
    1a4c:	7100001f 	cmp	w0, #0x0
    1a50:	540000a0 	b.eq	1a64 <main+0xec>  // b.none
        errExitEN(s, "pthread_create");
    1a54:	b0000000 	adrp	x0, 2000 <usageErr+0x10>
    1a58:	9113e001 	add	x1, x0, #0x4f8
    1a5c:	b9404be0 	ldr	w0, [sp, #72]
    1a60:	94000112 	bl	1ea8 <errExitEN>

    s = pthread_join(t1, NULL);
    1a64:	f94023e0 	ldr	x0, [sp, #64]
    1a68:	d2800001 	mov	x1, #0x0                   	// #0
    1a6c:	97ffff3d 	bl	1760 <pthread_join@plt>
    1a70:	b9004be0 	str	w0, [sp, #72]
    if (s != 0)
    1a74:	b9404be0 	ldr	w0, [sp, #72]
    1a78:	7100001f 	cmp	w0, #0x0
    1a7c:	540000a0 	b.eq	1a90 <main+0x118>  // b.none
        errExitEN(s, "pthread_join");
    1a80:	b0000000 	adrp	x0, 2000 <usageErr+0x10>
    1a84:	91142001 	add	x1, x0, #0x508
    1a88:	b9404be0 	ldr	w0, [sp, #72]
    1a8c:	94000107 	bl	1ea8 <errExitEN>
    s = pthread_join(t2, NULL);
    1a90:	f9401fe0 	ldr	x0, [sp, #56]
    1a94:	d2800001 	mov	x1, #0x0                   	// #0
    1a98:	97ffff32 	bl	1760 <pthread_join@plt>
    1a9c:	b9004be0 	str	w0, [sp, #72]
    if (s != 0)
    1aa0:	b9404be0 	ldr	w0, [sp, #72]
    1aa4:	7100001f 	cmp	w0, #0x0
    1aa8:	540000a0 	b.eq	1abc <main+0x144>  // b.none
        errExitEN(s, "pthread_join");
    1aac:	b0000000 	adrp	x0, 2000 <usageErr+0x10>
    1ab0:	91142001 	add	x1, x0, #0x508
    1ab4:	b9404be0 	ldr	w0, [sp, #72]
    1ab8:	940000fc 	bl	1ea8 <errExitEN>

    printf("glob = %d\n", glob);
    1abc:	f00000e0 	adrp	x0, 20000 <_exit@GLIBC_2.17>
    1ac0:	91139000 	add	x0, x0, #0x4e4
    1ac4:	b9400000 	ldr	w0, [x0]
    1ac8:	2a0003e1 	mov	w1, w0
    1acc:	b0000000 	adrp	x0, 2000 <usageErr+0x10>
    1ad0:	91146000 	add	x0, x0, #0x518
    1ad4:	97ffff1b 	bl	1740 <printf@plt>
    exit(EXIT_SUCCESS);
    1ad8:	52800000 	mov	w0, #0x0                   	// #0
    1adc:	97fffee5 	bl	1670 <exit@plt>

0000000000001ae0 <terminate>:
#include "ename.c.inc"          /* Defines ename and MAX_ENAME */

NORETURN
static void
terminate(Boolean useExit3)
{
    1ae0:	a9bd7bfd 	stp	x29, x30, [sp, #-48]!
    1ae4:	910003fd 	mov	x29, sp
    1ae8:	b9001fe0 	str	w0, [sp, #28]

    /* Dump core if EF_DUMPCORE environment variable is defined and
       is a nonempty string; otherwise call exit(3) or _exit(2),
       depending on the value of 'useExit3'. */

    s = getenv("EF_DUMPCORE");
    1aec:	b0000000 	adrp	x0, 2000 <usageErr+0x10>
    1af0:	912b6000 	add	x0, x0, #0xad8
    1af4:	97ffff1f 	bl	1770 <getenv@plt>
    1af8:	f90017e0 	str	x0, [sp, #40]

    if (s != NULL && *s != '\0')
    1afc:	f94017e0 	ldr	x0, [sp, #40]
    1b00:	f100001f 	cmp	x0, #0x0
    1b04:	540000c0 	b.eq	1b1c <terminate+0x3c>  // b.none
    1b08:	f94017e0 	ldr	x0, [sp, #40]
    1b0c:	39400000 	ldrb	w0, [x0]
    1b10:	7100001f 	cmp	w0, #0x0
    1b14:	54000040 	b.eq	1b1c <terminate+0x3c>  // b.none
        abort();
    1b18:	97fffeee 	bl	16d0 <abort@plt>
    else if (useExit3)
    1b1c:	b9401fe0 	ldr	w0, [sp, #28]
    1b20:	7100001f 	cmp	w0, #0x0
    1b24:	54000060 	b.eq	1b30 <terminate+0x50>  // b.none
        exit(EXIT_FAILURE);
    1b28:	52800020 	mov	w0, #0x1                   	// #1
    1b2c:	97fffed1 	bl	1670 <exit@plt>
    else
        _exit(EXIT_FAILURE);
    1b30:	52800020 	mov	w0, #0x1                   	// #1
    1b34:	97fffec7 	bl	1650 <_exit@plt>

0000000000001b38 <outputError>:
        'format' and 'ap'. */

static void
outputError(Boolean useErr, int err, Boolean flushStdout,
        const char *format, va_list ap)
{
    1b38:	d11943ff 	sub	sp, sp, #0x650
    1b3c:	a9007bfd 	stp	x29, x30, [sp]
    1b40:	910003fd 	mov	x29, sp
    1b44:	f9000bf3 	str	x19, [sp, #16]
    1b48:	b9005fe0 	str	w0, [sp, #92]
    1b4c:	b9005be1 	str	w1, [sp, #88]
    1b50:	b90057e2 	str	w2, [sp, #84]
    1b54:	f90027e3 	str	x3, [sp, #72]
    1b58:	aa0403f3 	mov	x19, x4
#define BUF_SIZE 500
    char buf[BUF_SIZE], userMsg[BUF_SIZE], errText[BUF_SIZE];

    vsnprintf(userMsg, BUF_SIZE, format, ap);
    1b5c:	910083e0 	add	x0, sp, #0x20
    1b60:	aa1303e1 	mov	x1, x19
    1b64:	ad400420 	ldp	q0, q1, [x1]
    1b68:	ad000400 	stp	q0, q1, [x0]
    1b6c:	910083e1 	add	x1, sp, #0x20
    1b70:	910983e0 	add	x0, sp, #0x260
    1b74:	aa0103e3 	mov	x3, x1
    1b78:	f94027e2 	ldr	x2, [sp, #72]
    1b7c:	d2803e81 	mov	x1, #0x1f4                 	// #500
    1b80:	97fffee8 	bl	1720 <vsnprintf@plt>

    if (useErr)
    1b84:	b9405fe0 	ldr	w0, [sp, #92]
    1b88:	7100001f 	cmp	w0, #0x0
    1b8c:	54000320 	b.eq	1bf0 <outputError+0xb8>  // b.none
        snprintf(errText, BUF_SIZE, " [%s %s]",
    1b90:	b9405be0 	ldr	w0, [sp, #88]
    1b94:	7100001f 	cmp	w0, #0x0
    1b98:	5400012d 	b.le	1bbc <outputError+0x84>
                (err > 0 && err <= MAX_ENAME) ?
    1b9c:	b9405be0 	ldr	w0, [sp, #88]
    1ba0:	7102141f 	cmp	w0, #0x85
    1ba4:	540000cc 	b.gt	1bbc <outputError+0x84>
        snprintf(errText, BUF_SIZE, " [%s %s]",
    1ba8:	f00000e0 	adrp	x0, 20000 <_exit@GLIBC_2.17>
    1bac:	9102c000 	add	x0, x0, #0xb0
    1bb0:	b9805be1 	ldrsw	x1, [sp, #88]
    1bb4:	f8617813 	ldr	x19, [x0, x1, lsl #3]
    1bb8:	14000003 	b	1bc4 <outputError+0x8c>
    1bbc:	b0000000 	adrp	x0, 2000 <usageErr+0x10>
    1bc0:	912ba013 	add	x19, x0, #0xae8
    1bc4:	b9405be0 	ldr	w0, [sp, #88]
    1bc8:	97fffeba 	bl	16b0 <strerror@plt>
    1bcc:	9101a3e5 	add	x5, sp, #0x68
    1bd0:	aa0003e4 	mov	x4, x0
    1bd4:	aa1303e3 	mov	x3, x19
    1bd8:	b0000000 	adrp	x0, 2000 <usageErr+0x10>
    1bdc:	912be002 	add	x2, x0, #0xaf8
    1be0:	d2803e81 	mov	x1, #0x1f4                 	// #500
    1be4:	aa0503e0 	mov	x0, x5
    1be8:	97fffeae 	bl	16a0 <snprintf@plt>
    1bec:	14000007 	b	1c08 <outputError+0xd0>
                ename[err] : "?UNKNOWN?", strerror(err));
    else
        snprintf(errText, BUF_SIZE, ":");
    1bf0:	9101a3e3 	add	x3, sp, #0x68
    1bf4:	b0000000 	adrp	x0, 2000 <usageErr+0x10>
    1bf8:	912c2002 	add	x2, x0, #0xb08
    1bfc:	d2803e81 	mov	x1, #0x1f4                 	// #500
    1c00:	aa0303e0 	mov	x0, x3
    1c04:	97fffea7 	bl	16a0 <snprintf@plt>

#if __GNUC__ >= 7
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
    snprintf(buf, BUF_SIZE, "ERROR%s %s\n", errText, userMsg);
    1c08:	910983e1 	add	x1, sp, #0x260
    1c0c:	9101a3e0 	add	x0, sp, #0x68
    1c10:	911163e5 	add	x5, sp, #0x458
    1c14:	aa0103e4 	mov	x4, x1
    1c18:	aa0003e3 	mov	x3, x0
    1c1c:	b0000000 	adrp	x0, 2000 <usageErr+0x10>
    1c20:	912c4002 	add	x2, x0, #0xb10
    1c24:	d2803e81 	mov	x1, #0x1f4                 	// #500
    1c28:	aa0503e0 	mov	x0, x5
    1c2c:	97fffe9d 	bl	16a0 <snprintf@plt>
#if __GNUC__ >= 7
#pragma GCC diagnostic pop
#endif

    if (flushStdout)
    1c30:	b94057e0 	ldr	w0, [sp, #84]
    1c34:	7100001f 	cmp	w0, #0x0
    1c38:	540000a0 	b.eq	1c4c <outputError+0x114>  // b.none
        fflush(stdout);       /* Flush any pending stdout */
    1c3c:	d00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    1c40:	f947e400 	ldr	x0, [x0, #4040]
    1c44:	f9400000 	ldr	x0, [x0]
    1c48:	97fffeb2 	bl	1710 <fflush@plt>
    fputs(buf, stderr);
    1c4c:	d00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    1c50:	f947e000 	ldr	x0, [x0, #4032]
    1c54:	f9400001 	ldr	x1, [x0]
    1c58:	911163e0 	add	x0, sp, #0x458
    1c5c:	97fffe81 	bl	1660 <fputs@plt>
    fflush(stderr);           /* In case stderr is not line-buffered */
    1c60:	d00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    1c64:	f947e000 	ldr	x0, [x0, #4032]
    1c68:	f9400000 	ldr	x0, [x0]
    1c6c:	97fffea9 	bl	1710 <fflush@plt>
}
    1c70:	d503201f 	nop
    1c74:	f9400bf3 	ldr	x19, [sp, #16]
    1c78:	a9407bfd 	ldp	x29, x30, [sp]
    1c7c:	911943ff 	add	sp, sp, #0x650
    1c80:	d65f03c0 	ret

0000000000001c84 <errMsg>:
/* Display error message including 'errno' diagnostic, and
   return to caller */

void
errMsg(const char *format, ...)
{
    1c84:	a9ad7bfd 	stp	x29, x30, [sp, #-304]!
    1c88:	910003fd 	mov	x29, sp
    1c8c:	f9001fe0 	str	x0, [sp, #56]
    1c90:	f9007fe1 	str	x1, [sp, #248]
    1c94:	f90083e2 	str	x2, [sp, #256]
    1c98:	f90087e3 	str	x3, [sp, #264]
    1c9c:	f9008be4 	str	x4, [sp, #272]
    1ca0:	f9008fe5 	str	x5, [sp, #280]
    1ca4:	f90093e6 	str	x6, [sp, #288]
    1ca8:	f90097e7 	str	x7, [sp, #296]
    1cac:	3d801fe0 	str	q0, [sp, #112]
    1cb0:	3d8023e1 	str	q1, [sp, #128]
    1cb4:	3d8027e2 	str	q2, [sp, #144]
    1cb8:	3d802be3 	str	q3, [sp, #160]
    1cbc:	3d802fe4 	str	q4, [sp, #176]
    1cc0:	3d8033e5 	str	q5, [sp, #192]
    1cc4:	3d8037e6 	str	q6, [sp, #208]
    1cc8:	3d803be7 	str	q7, [sp, #224]
    va_list argList;
    int savedErrno;

    savedErrno = errno;       /* In case we change it here */
    1ccc:	97fffea1 	bl	1750 <__errno_location@plt>
    1cd0:	b9400000 	ldr	w0, [x0]
    1cd4:	b9006fe0 	str	w0, [sp, #108]

    va_start(argList, format);
    1cd8:	9104c3e0 	add	x0, sp, #0x130
    1cdc:	f90027e0 	str	x0, [sp, #72]
    1ce0:	9104c3e0 	add	x0, sp, #0x130
    1ce4:	f9002be0 	str	x0, [sp, #80]
    1ce8:	9103c3e0 	add	x0, sp, #0xf0
    1cec:	f9002fe0 	str	x0, [sp, #88]
    1cf0:	128006e0 	mov	w0, #0xffffffc8            	// #-56
    1cf4:	b90063e0 	str	w0, [sp, #96]
    1cf8:	12800fe0 	mov	w0, #0xffffff80            	// #-128
    1cfc:	b90067e0 	str	w0, [sp, #100]
    outputError(TRUE, errno, TRUE, format, argList);
    1d00:	97fffe94 	bl	1750 <__errno_location@plt>
    1d04:	b9400005 	ldr	w5, [x0]
    1d08:	910043e0 	add	x0, sp, #0x10
    1d0c:	910123e1 	add	x1, sp, #0x48
    1d10:	ad400420 	ldp	q0, q1, [x1]
    1d14:	ad000400 	stp	q0, q1, [x0]
    1d18:	910043e0 	add	x0, sp, #0x10
    1d1c:	aa0003e4 	mov	x4, x0
    1d20:	f9401fe3 	ldr	x3, [sp, #56]
    1d24:	52800022 	mov	w2, #0x1                   	// #1
    1d28:	2a0503e1 	mov	w1, w5
    1d2c:	52800020 	mov	w0, #0x1                   	// #1
    1d30:	97ffff82 	bl	1b38 <outputError>
    va_end(argList);

    errno = savedErrno;
    1d34:	97fffe87 	bl	1750 <__errno_location@plt>
    1d38:	aa0003e1 	mov	x1, x0
    1d3c:	b9406fe0 	ldr	w0, [sp, #108]
    1d40:	b9000020 	str	w0, [x1]
}
    1d44:	d503201f 	nop
    1d48:	a8d37bfd 	ldp	x29, x30, [sp], #304
    1d4c:	d65f03c0 	ret

0000000000001d50 <errExit>:
/* Display error message including 'errno' diagnostic, and
   terminate the process */

void
errExit(const char *format, ...)
{
    1d50:	a9ae7bfd 	stp	x29, x30, [sp, #-288]!
    1d54:	910003fd 	mov	x29, sp
    1d58:	f9001fe0 	str	x0, [sp, #56]
    1d5c:	f90077e1 	str	x1, [sp, #232]
    1d60:	f9007be2 	str	x2, [sp, #240]
    1d64:	f9007fe3 	str	x3, [sp, #248]
    1d68:	f90083e4 	str	x4, [sp, #256]
    1d6c:	f90087e5 	str	x5, [sp, #264]
    1d70:	f9008be6 	str	x6, [sp, #272]
    1d74:	f9008fe7 	str	x7, [sp, #280]
    1d78:	3d801be0 	str	q0, [sp, #96]
    1d7c:	3d801fe1 	str	q1, [sp, #112]
    1d80:	3d8023e2 	str	q2, [sp, #128]
    1d84:	3d8027e3 	str	q3, [sp, #144]
    1d88:	3d802be4 	str	q4, [sp, #160]
    1d8c:	3d802fe5 	str	q5, [sp, #176]
    1d90:	3d8033e6 	str	q6, [sp, #192]
    1d94:	3d8037e7 	str	q7, [sp, #208]
    va_list argList;

    va_start(argList, format);
    1d98:	910483e0 	add	x0, sp, #0x120
    1d9c:	f90023e0 	str	x0, [sp, #64]
    1da0:	910483e0 	add	x0, sp, #0x120
    1da4:	f90027e0 	str	x0, [sp, #72]
    1da8:	910383e0 	add	x0, sp, #0xe0
    1dac:	f9002be0 	str	x0, [sp, #80]
    1db0:	128006e0 	mov	w0, #0xffffffc8            	// #-56
    1db4:	b9005be0 	str	w0, [sp, #88]
    1db8:	12800fe0 	mov	w0, #0xffffff80            	// #-128
    1dbc:	b9005fe0 	str	w0, [sp, #92]
    outputError(TRUE, errno, TRUE, format, argList);
    1dc0:	97fffe64 	bl	1750 <__errno_location@plt>
    1dc4:	b9400005 	ldr	w5, [x0]
    1dc8:	910043e0 	add	x0, sp, #0x10
    1dcc:	910103e1 	add	x1, sp, #0x40
    1dd0:	ad400420 	ldp	q0, q1, [x1]
    1dd4:	ad000400 	stp	q0, q1, [x0]
    1dd8:	910043e0 	add	x0, sp, #0x10
    1ddc:	aa0003e4 	mov	x4, x0
    1de0:	f9401fe3 	ldr	x3, [sp, #56]
    1de4:	52800022 	mov	w2, #0x1                   	// #1
    1de8:	2a0503e1 	mov	w1, w5
    1dec:	52800020 	mov	w0, #0x1                   	// #1
    1df0:	97ffff52 	bl	1b38 <outputError>
    va_end(argList);

    terminate(TRUE);
    1df4:	52800020 	mov	w0, #0x1                   	// #1
    1df8:	97ffff3a 	bl	1ae0 <terminate>

0000000000001dfc <err_exit>:
   stdio buffers that were partially filled by the caller and without
   invoking exit handlers that were established by the caller. */

void
err_exit(const char *format, ...)
{
    1dfc:	a9ae7bfd 	stp	x29, x30, [sp, #-288]!
    1e00:	910003fd 	mov	x29, sp
    1e04:	f9001fe0 	str	x0, [sp, #56]
    1e08:	f90077e1 	str	x1, [sp, #232]
    1e0c:	f9007be2 	str	x2, [sp, #240]
    1e10:	f9007fe3 	str	x3, [sp, #248]
    1e14:	f90083e4 	str	x4, [sp, #256]
    1e18:	f90087e5 	str	x5, [sp, #264]
    1e1c:	f9008be6 	str	x6, [sp, #272]
    1e20:	f9008fe7 	str	x7, [sp, #280]
    1e24:	3d801be0 	str	q0, [sp, #96]
    1e28:	3d801fe1 	str	q1, [sp, #112]
    1e2c:	3d8023e2 	str	q2, [sp, #128]
    1e30:	3d8027e3 	str	q3, [sp, #144]
    1e34:	3d802be4 	str	q4, [sp, #160]
    1e38:	3d802fe5 	str	q5, [sp, #176]
    1e3c:	3d8033e6 	str	q6, [sp, #192]
    1e40:	3d8037e7 	str	q7, [sp, #208]
    va_list argList;

    va_start(argList, format);
    1e44:	910483e0 	add	x0, sp, #0x120
    1e48:	f90023e0 	str	x0, [sp, #64]
    1e4c:	910483e0 	add	x0, sp, #0x120
    1e50:	f90027e0 	str	x0, [sp, #72]
    1e54:	910383e0 	add	x0, sp, #0xe0
    1e58:	f9002be0 	str	x0, [sp, #80]
    1e5c:	128006e0 	mov	w0, #0xffffffc8            	// #-56
    1e60:	b9005be0 	str	w0, [sp, #88]
    1e64:	12800fe0 	mov	w0, #0xffffff80            	// #-128
    1e68:	b9005fe0 	str	w0, [sp, #92]
    outputError(TRUE, errno, FALSE, format, argList);
    1e6c:	97fffe39 	bl	1750 <__errno_location@plt>
    1e70:	b9400005 	ldr	w5, [x0]
    1e74:	910043e0 	add	x0, sp, #0x10
    1e78:	910103e1 	add	x1, sp, #0x40
    1e7c:	ad400420 	ldp	q0, q1, [x1]
    1e80:	ad000400 	stp	q0, q1, [x0]
    1e84:	910043e0 	add	x0, sp, #0x10
    1e88:	aa0003e4 	mov	x4, x0
    1e8c:	f9401fe3 	ldr	x3, [sp, #56]
    1e90:	52800002 	mov	w2, #0x0                   	// #0
    1e94:	2a0503e1 	mov	w1, w5
    1e98:	52800020 	mov	w0, #0x1                   	// #1
    1e9c:	97ffff27 	bl	1b38 <outputError>
    va_end(argList);

    terminate(FALSE);
    1ea0:	52800000 	mov	w0, #0x0                   	// #0
    1ea4:	97ffff0f 	bl	1ae0 <terminate>

0000000000001ea8 <errExitEN>:
/* The following function does the same as errExit(), but expects
   the error number in 'errnum' */

void
errExitEN(int errnum, const char *format, ...)
{
    1ea8:	a9af7bfd 	stp	x29, x30, [sp, #-272]!
    1eac:	910003fd 	mov	x29, sp
    1eb0:	b9003fe0 	str	w0, [sp, #60]
    1eb4:	f9001be1 	str	x1, [sp, #48]
    1eb8:	f90073e2 	str	x2, [sp, #224]
    1ebc:	f90077e3 	str	x3, [sp, #232]
    1ec0:	f9007be4 	str	x4, [sp, #240]
    1ec4:	f9007fe5 	str	x5, [sp, #248]
    1ec8:	f90083e6 	str	x6, [sp, #256]
    1ecc:	f90087e7 	str	x7, [sp, #264]
    1ed0:	3d801be0 	str	q0, [sp, #96]
    1ed4:	3d801fe1 	str	q1, [sp, #112]
    1ed8:	3d8023e2 	str	q2, [sp, #128]
    1edc:	3d8027e3 	str	q3, [sp, #144]
    1ee0:	3d802be4 	str	q4, [sp, #160]
    1ee4:	3d802fe5 	str	q5, [sp, #176]
    1ee8:	3d8033e6 	str	q6, [sp, #192]
    1eec:	3d8037e7 	str	q7, [sp, #208]
    va_list argList;

    va_start(argList, format);
    1ef0:	910443e0 	add	x0, sp, #0x110
    1ef4:	f90023e0 	str	x0, [sp, #64]
    1ef8:	910443e0 	add	x0, sp, #0x110
    1efc:	f90027e0 	str	x0, [sp, #72]
    1f00:	910383e0 	add	x0, sp, #0xe0
    1f04:	f9002be0 	str	x0, [sp, #80]
    1f08:	128005e0 	mov	w0, #0xffffffd0            	// #-48
    1f0c:	b9005be0 	str	w0, [sp, #88]
    1f10:	12800fe0 	mov	w0, #0xffffff80            	// #-128
    1f14:	b9005fe0 	str	w0, [sp, #92]
    outputError(TRUE, errnum, TRUE, format, argList);
    1f18:	910043e0 	add	x0, sp, #0x10
    1f1c:	910103e1 	add	x1, sp, #0x40
    1f20:	ad400420 	ldp	q0, q1, [x1]
    1f24:	ad000400 	stp	q0, q1, [x0]
    1f28:	910043e0 	add	x0, sp, #0x10
    1f2c:	aa0003e4 	mov	x4, x0
    1f30:	f9401be3 	ldr	x3, [sp, #48]
    1f34:	52800022 	mov	w2, #0x1                   	// #1
    1f38:	b9403fe1 	ldr	w1, [sp, #60]
    1f3c:	52800020 	mov	w0, #0x1                   	// #1
    1f40:	97fffefe 	bl	1b38 <outputError>
    va_end(argList);

    terminate(TRUE);
    1f44:	52800020 	mov	w0, #0x1                   	// #1
    1f48:	97fffee6 	bl	1ae0 <terminate>

0000000000001f4c <fatal>:

/* Print an error message (without an 'errno' diagnostic) */

void
fatal(const char *format, ...)
{
    1f4c:	a9ae7bfd 	stp	x29, x30, [sp, #-288]!
    1f50:	910003fd 	mov	x29, sp
    1f54:	f9001fe0 	str	x0, [sp, #56]
    1f58:	f90077e1 	str	x1, [sp, #232]
    1f5c:	f9007be2 	str	x2, [sp, #240]
    1f60:	f9007fe3 	str	x3, [sp, #248]
    1f64:	f90083e4 	str	x4, [sp, #256]
    1f68:	f90087e5 	str	x5, [sp, #264]
    1f6c:	f9008be6 	str	x6, [sp, #272]
    1f70:	f9008fe7 	str	x7, [sp, #280]
    1f74:	3d801be0 	str	q0, [sp, #96]
    1f78:	3d801fe1 	str	q1, [sp, #112]
    1f7c:	3d8023e2 	str	q2, [sp, #128]
    1f80:	3d8027e3 	str	q3, [sp, #144]
    1f84:	3d802be4 	str	q4, [sp, #160]
    1f88:	3d802fe5 	str	q5, [sp, #176]
    1f8c:	3d8033e6 	str	q6, [sp, #192]
    1f90:	3d8037e7 	str	q7, [sp, #208]
    va_list argList;

    va_start(argList, format);
    1f94:	910483e0 	add	x0, sp, #0x120
    1f98:	f90023e0 	str	x0, [sp, #64]
    1f9c:	910483e0 	add	x0, sp, #0x120
    1fa0:	f90027e0 	str	x0, [sp, #72]
    1fa4:	910383e0 	add	x0, sp, #0xe0
    1fa8:	f9002be0 	str	x0, [sp, #80]
    1fac:	128006e0 	mov	w0, #0xffffffc8            	// #-56
    1fb0:	b9005be0 	str	w0, [sp, #88]
    1fb4:	12800fe0 	mov	w0, #0xffffff80            	// #-128
    1fb8:	b9005fe0 	str	w0, [sp, #92]
    outputError(FALSE, 0, TRUE, format, argList);
    1fbc:	910043e0 	add	x0, sp, #0x10
    1fc0:	910103e1 	add	x1, sp, #0x40
    1fc4:	ad400420 	ldp	q0, q1, [x1]
    1fc8:	ad000400 	stp	q0, q1, [x0]
    1fcc:	910043e0 	add	x0, sp, #0x10
    1fd0:	aa0003e4 	mov	x4, x0
    1fd4:	f9401fe3 	ldr	x3, [sp, #56]
    1fd8:	52800022 	mov	w2, #0x1                   	// #1
    1fdc:	52800001 	mov	w1, #0x0                   	// #0
    1fe0:	52800000 	mov	w0, #0x0                   	// #0
    1fe4:	97fffed5 	bl	1b38 <outputError>
    va_end(argList);

    terminate(TRUE);
    1fe8:	52800020 	mov	w0, #0x1                   	// #1
    1fec:	97fffebd 	bl	1ae0 <terminate>

0000000000001ff0 <usageErr>:

/* Print a command usage error message and terminate the process */

void
usageErr(const char *format, ...)
{
    1ff0:	a9ae7bfd 	stp	x29, x30, [sp, #-288]!
    1ff4:	910003fd 	mov	x29, sp
    1ff8:	f9001fe0 	str	x0, [sp, #56]
    1ffc:	f90077e1 	str	x1, [sp, #232]
    2000:	f9007be2 	str	x2, [sp, #240]
    2004:	f9007fe3 	str	x3, [sp, #248]
    2008:	f90083e4 	str	x4, [sp, #256]
    200c:	f90087e5 	str	x5, [sp, #264]
    2010:	f9008be6 	str	x6, [sp, #272]
    2014:	f9008fe7 	str	x7, [sp, #280]
    2018:	3d801be0 	str	q0, [sp, #96]
    201c:	3d801fe1 	str	q1, [sp, #112]
    2020:	3d8023e2 	str	q2, [sp, #128]
    2024:	3d8027e3 	str	q3, [sp, #144]
    2028:	3d802be4 	str	q4, [sp, #160]
    202c:	3d802fe5 	str	q5, [sp, #176]
    2030:	3d8033e6 	str	q6, [sp, #192]
    2034:	3d8037e7 	str	q7, [sp, #208]
    va_list argList;

    fflush(stdout);           /* Flush any pending stdout */
    2038:	b00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    203c:	f947e400 	ldr	x0, [x0, #4040]
    2040:	f9400000 	ldr	x0, [x0]
    2044:	97fffdb3 	bl	1710 <fflush@plt>

    fprintf(stderr, "Usage: ");
    2048:	b00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    204c:	f947e000 	ldr	x0, [x0, #4032]
    2050:	f9400000 	ldr	x0, [x0]
    2054:	aa0003e3 	mov	x3, x0
    2058:	d28000e2 	mov	x2, #0x7                   	// #7
    205c:	d2800021 	mov	x1, #0x1                   	// #1
    2060:	90000000 	adrp	x0, 2000 <usageErr+0x10>
    2064:	912c8000 	add	x0, x0, #0xb20
    2068:	97fffda2 	bl	16f0 <fwrite@plt>
    va_start(argList, format);
    206c:	910483e0 	add	x0, sp, #0x120
    2070:	f90023e0 	str	x0, [sp, #64]
    2074:	910483e0 	add	x0, sp, #0x120
    2078:	f90027e0 	str	x0, [sp, #72]
    207c:	910383e0 	add	x0, sp, #0xe0
    2080:	f9002be0 	str	x0, [sp, #80]
    2084:	128006e0 	mov	w0, #0xffffffc8            	// #-56
    2088:	b9005be0 	str	w0, [sp, #88]
    208c:	12800fe0 	mov	w0, #0xffffff80            	// #-128
    2090:	b9005fe0 	str	w0, [sp, #92]
    vfprintf(stderr, format, argList);
    2094:	b00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    2098:	f947e000 	ldr	x0, [x0, #4032]
    209c:	f9400003 	ldr	x3, [x0]
    20a0:	910043e0 	add	x0, sp, #0x10
    20a4:	910103e1 	add	x1, sp, #0x40
    20a8:	ad400420 	ldp	q0, q1, [x1]
    20ac:	ad000400 	stp	q0, q1, [x0]
    20b0:	910043e0 	add	x0, sp, #0x10
    20b4:	aa0003e2 	mov	x2, x0
    20b8:	f9401fe1 	ldr	x1, [sp, #56]
    20bc:	aa0303e0 	mov	x0, x3
    20c0:	97fffd9c 	bl	1730 <vfprintf@plt>
    va_end(argList);

    fflush(stderr);           /* In case stderr is not line-buffered */
    20c4:	b00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    20c8:	f947e000 	ldr	x0, [x0, #4032]
    20cc:	f9400000 	ldr	x0, [x0]
    20d0:	97fffd90 	bl	1710 <fflush@plt>
    exit(EXIT_FAILURE);
    20d4:	52800020 	mov	w0, #0x1                   	// #1
    20d8:	97fffd66 	bl	1670 <exit@plt>

00000000000020dc <cmdLineErr>:
/* Diagnose an error in command-line arguments and
   terminate the process */

void
cmdLineErr(const char *format, ...)
{
    20dc:	a9ae7bfd 	stp	x29, x30, [sp, #-288]!
    20e0:	910003fd 	mov	x29, sp
    20e4:	f9001fe0 	str	x0, [sp, #56]
    20e8:	f90077e1 	str	x1, [sp, #232]
    20ec:	f9007be2 	str	x2, [sp, #240]
    20f0:	f9007fe3 	str	x3, [sp, #248]
    20f4:	f90083e4 	str	x4, [sp, #256]
    20f8:	f90087e5 	str	x5, [sp, #264]
    20fc:	f9008be6 	str	x6, [sp, #272]
    2100:	f9008fe7 	str	x7, [sp, #280]
    2104:	3d801be0 	str	q0, [sp, #96]
    2108:	3d801fe1 	str	q1, [sp, #112]
    210c:	3d8023e2 	str	q2, [sp, #128]
    2110:	3d8027e3 	str	q3, [sp, #144]
    2114:	3d802be4 	str	q4, [sp, #160]
    2118:	3d802fe5 	str	q5, [sp, #176]
    211c:	3d8033e6 	str	q6, [sp, #192]
    2120:	3d8037e7 	str	q7, [sp, #208]
    va_list argList;

    fflush(stdout);           /* Flush any pending stdout */
    2124:	b00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    2128:	f947e400 	ldr	x0, [x0, #4040]
    212c:	f9400000 	ldr	x0, [x0]
    2130:	97fffd78 	bl	1710 <fflush@plt>

    fprintf(stderr, "Command-line usage error: ");
    2134:	b00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    2138:	f947e000 	ldr	x0, [x0, #4032]
    213c:	f9400000 	ldr	x0, [x0]
    2140:	aa0003e3 	mov	x3, x0
    2144:	d2800342 	mov	x2, #0x1a                  	// #26
    2148:	d2800021 	mov	x1, #0x1                   	// #1
    214c:	90000000 	adrp	x0, 2000 <usageErr+0x10>
    2150:	912ca000 	add	x0, x0, #0xb28
    2154:	97fffd67 	bl	16f0 <fwrite@plt>
    va_start(argList, format);
    2158:	910483e0 	add	x0, sp, #0x120
    215c:	f90023e0 	str	x0, [sp, #64]
    2160:	910483e0 	add	x0, sp, #0x120
    2164:	f90027e0 	str	x0, [sp, #72]
    2168:	910383e0 	add	x0, sp, #0xe0
    216c:	f9002be0 	str	x0, [sp, #80]
    2170:	128006e0 	mov	w0, #0xffffffc8            	// #-56
    2174:	b9005be0 	str	w0, [sp, #88]
    2178:	12800fe0 	mov	w0, #0xffffff80            	// #-128
    217c:	b9005fe0 	str	w0, [sp, #92]
    vfprintf(stderr, format, argList);
    2180:	b00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    2184:	f947e000 	ldr	x0, [x0, #4032]
    2188:	f9400003 	ldr	x3, [x0]
    218c:	910043e0 	add	x0, sp, #0x10
    2190:	910103e1 	add	x1, sp, #0x40
    2194:	ad400420 	ldp	q0, q1, [x1]
    2198:	ad000400 	stp	q0, q1, [x0]
    219c:	910043e0 	add	x0, sp, #0x10
    21a0:	aa0003e2 	mov	x2, x0
    21a4:	f9401fe1 	ldr	x1, [sp, #56]
    21a8:	aa0303e0 	mov	x0, x3
    21ac:	97fffd61 	bl	1730 <vfprintf@plt>
    va_end(argList);

    fflush(stderr);           /* In case stderr is not line-buffered */
    21b0:	b00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    21b4:	f947e000 	ldr	x0, [x0, #4032]
    21b8:	f9400000 	ldr	x0, [x0]
    21bc:	97fffd55 	bl	1710 <fflush@plt>
    exit(EXIT_FAILURE);
    21c0:	52800020 	mov	w0, #0x1                   	// #1
    21c4:	97fffd2b 	bl	1670 <exit@plt>

00000000000021c8 <gnFail>:
/* Print a diagnostic message that contains a function name ('fname'),
   the value of a command-line argument ('arg'), the name of that
   command-line argument ('name'), and a diagnostic error message ('msg'). */
static void
gnFail(const char *fname, const char *msg, const char *arg, const char *name)
{
    21c8:	a9bd7bfd 	stp	x29, x30, [sp, #-48]!
    21cc:	910003fd 	mov	x29, sp
    21d0:	f90017e0 	str	x0, [sp, #40]
    21d4:	f90013e1 	str	x1, [sp, #32]
    21d8:	f9000fe2 	str	x2, [sp, #24]
    21dc:	f9000be3 	str	x3, [sp, #16]
    fprintf(stderr, "%s error", fname);
    21e0:	b00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    21e4:	f947e000 	ldr	x0, [x0, #4032]
    21e8:	f9400003 	ldr	x3, [x0]
    21ec:	f94017e2 	ldr	x2, [sp, #40]
    21f0:	90000000 	adrp	x0, 2000 <usageErr+0x10>
    21f4:	912d2001 	add	x1, x0, #0xb48
    21f8:	aa0303e0 	mov	x0, x3
    21fc:	97fffd61 	bl	1780 <fprintf@plt>
    if (name != NULL)
    2200:	f9400be0 	ldr	x0, [sp, #16]
    2204:	f100001f 	cmp	x0, #0x0
    2208:	54000120 	b.eq	222c <gnFail+0x64>  // b.none
        fprintf(stderr, " (in %s)", name);
    220c:	b00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    2210:	f947e000 	ldr	x0, [x0, #4032]
    2214:	f9400003 	ldr	x3, [x0]
    2218:	f9400be2 	ldr	x2, [sp, #16]
    221c:	90000000 	adrp	x0, 2000 <usageErr+0x10>
    2220:	912d6001 	add	x1, x0, #0xb58
    2224:	aa0303e0 	mov	x0, x3
    2228:	97fffd56 	bl	1780 <fprintf@plt>
    fprintf(stderr, ": %s\n", msg);
    222c:	b00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    2230:	f947e000 	ldr	x0, [x0, #4032]
    2234:	f9400003 	ldr	x3, [x0]
    2238:	f94013e2 	ldr	x2, [sp, #32]
    223c:	90000000 	adrp	x0, 2000 <usageErr+0x10>
    2240:	912da001 	add	x1, x0, #0xb68
    2244:	aa0303e0 	mov	x0, x3
    2248:	97fffd4e 	bl	1780 <fprintf@plt>
    if (arg != NULL && *arg != '\0')
    224c:	f9400fe0 	ldr	x0, [sp, #24]
    2250:	f100001f 	cmp	x0, #0x0
    2254:	540001a0 	b.eq	2288 <gnFail+0xc0>  // b.none
    2258:	f9400fe0 	ldr	x0, [sp, #24]
    225c:	39400000 	ldrb	w0, [x0]
    2260:	7100001f 	cmp	w0, #0x0
    2264:	54000120 	b.eq	2288 <gnFail+0xc0>  // b.none
        fprintf(stderr, "        offending text: %s\n", arg);
    2268:	b00000e0 	adrp	x0, 1f000 <__FRAME_END__+0x1c0e0>
    226c:	f947e000 	ldr	x0, [x0, #4032]
    2270:	f9400003 	ldr	x3, [x0]
    2274:	f9400fe2 	ldr	x2, [sp, #24]
    2278:	90000000 	adrp	x0, 2000 <usageErr+0x10>
    227c:	912dc001 	add	x1, x0, #0xb70
    2280:	aa0303e0 	mov	x0, x3
    2284:	97fffd3f 	bl	1780 <fprintf@plt>

    exit(EXIT_FAILURE);
    2288:	52800020 	mov	w0, #0x1                   	// #1
    228c:	97fffcf9 	bl	1670 <exit@plt>

0000000000002290 <getNum>:
   the command-line argument 'arg'. 'fname' and 'name' are used to print a
   diagnostic message in case an error is detected when processing 'arg'. */

static long
getNum(const char *fname, const char *arg, int flags, const char *name)
{
    2290:	a9bb7bfd 	stp	x29, x30, [sp, #-80]!
    2294:	910003fd 	mov	x29, sp
    2298:	f90017e0 	str	x0, [sp, #40]
    229c:	f90013e1 	str	x1, [sp, #32]
    22a0:	b9001fe2 	str	w2, [sp, #28]
    22a4:	f9000be3 	str	x3, [sp, #16]
    long res;
    char *endptr;
    int base;

    if (arg == NULL || *arg == '\0')
    22a8:	f94013e0 	ldr	x0, [sp, #32]
    22ac:	f100001f 	cmp	x0, #0x0
    22b0:	540000a0 	b.eq	22c4 <getNum+0x34>  // b.none
    22b4:	f94013e0 	ldr	x0, [sp, #32]
    22b8:	39400000 	ldrb	w0, [x0]
    22bc:	7100001f 	cmp	w0, #0x0
    22c0:	540000e1 	b.ne	22dc <getNum+0x4c>  // b.any
        gnFail(fname, "null or empty string", arg, name);
    22c4:	f9400be3 	ldr	x3, [sp, #16]
    22c8:	f94013e2 	ldr	x2, [sp, #32]
    22cc:	90000000 	adrp	x0, 2000 <usageErr+0x10>
    22d0:	912e4001 	add	x1, x0, #0xb90
    22d4:	f94017e0 	ldr	x0, [sp, #40]
    22d8:	97ffffbc 	bl	21c8 <gnFail>

    base = (flags & GN_ANY_BASE) ? 0 : (flags & GN_BASE_8) ? 8 :
    22dc:	b9401fe0 	ldr	w0, [sp, #28]
    22e0:	121a0000 	and	w0, w0, #0x40
    22e4:	7100001f 	cmp	w0, #0x0
    22e8:	540001e1 	b.ne	2324 <getNum+0x94>  // b.any
    22ec:	b9401fe0 	ldr	w0, [sp, #28]
    22f0:	12190000 	and	w0, w0, #0x80
    22f4:	7100001f 	cmp	w0, #0x0
    22f8:	54000121 	b.ne	231c <getNum+0x8c>  // b.any
                        (flags & GN_BASE_16) ? 16 : 10;
    22fc:	b9401fe0 	ldr	w0, [sp, #28]
    2300:	12180000 	and	w0, w0, #0x100
    2304:	7100001f 	cmp	w0, #0x0
    2308:	54000060 	b.eq	2314 <getNum+0x84>  // b.none
    230c:	52800200 	mov	w0, #0x10                  	// #16
    2310:	14000006 	b	2328 <getNum+0x98>
    2314:	52800140 	mov	w0, #0xa                   	// #10
    2318:	14000004 	b	2328 <getNum+0x98>
    base = (flags & GN_ANY_BASE) ? 0 : (flags & GN_BASE_8) ? 8 :
    231c:	52800100 	mov	w0, #0x8                   	// #8
    2320:	14000002 	b	2328 <getNum+0x98>
    2324:	52800000 	mov	w0, #0x0                   	// #0
    2328:	b9004fe0 	str	w0, [sp, #76]

    errno = 0;
    232c:	97fffd09 	bl	1750 <__errno_location@plt>
    2330:	b900001f 	str	wzr, [x0]
    res = strtol(arg, &endptr, base);
    2334:	9100e3e0 	add	x0, sp, #0x38
    2338:	b9404fe2 	ldr	w2, [sp, #76]
    233c:	aa0003e1 	mov	x1, x0
    2340:	f94013e0 	ldr	x0, [sp, #32]
    2344:	97fffce7 	bl	16e0 <strtol@plt>
    2348:	f90023e0 	str	x0, [sp, #64]
    if (errno != 0)
    234c:	97fffd01 	bl	1750 <__errno_location@plt>
    2350:	b9400000 	ldr	w0, [x0]
    2354:	7100001f 	cmp	w0, #0x0
    2358:	540000e0 	b.eq	2374 <getNum+0xe4>  // b.none
        gnFail(fname, "strtol() failed", arg, name);
    235c:	f9400be3 	ldr	x3, [sp, #16]
    2360:	f94013e2 	ldr	x2, [sp, #32]
    2364:	90000000 	adrp	x0, 2000 <usageErr+0x10>
    2368:	912ea001 	add	x1, x0, #0xba8
    236c:	f94017e0 	ldr	x0, [sp, #40]
    2370:	97ffff96 	bl	21c8 <gnFail>

    if (*endptr != '\0')
    2374:	f9401fe0 	ldr	x0, [sp, #56]
    2378:	39400000 	ldrb	w0, [x0]
    237c:	7100001f 	cmp	w0, #0x0
    2380:	540000e0 	b.eq	239c <getNum+0x10c>  // b.none
        gnFail(fname, "nonnumeric characters", arg, name);
    2384:	f9400be3 	ldr	x3, [sp, #16]
    2388:	f94013e2 	ldr	x2, [sp, #32]
    238c:	90000000 	adrp	x0, 2000 <usageErr+0x10>
    2390:	912ee001 	add	x1, x0, #0xbb8
    2394:	f94017e0 	ldr	x0, [sp, #40]
    2398:	97ffff8c 	bl	21c8 <gnFail>

    if ((flags & GN_NONNEG) && res < 0)
    239c:	b9401fe0 	ldr	w0, [sp, #28]
    23a0:	12000000 	and	w0, w0, #0x1
    23a4:	7100001f 	cmp	w0, #0x0
    23a8:	54000140 	b.eq	23d0 <getNum+0x140>  // b.none
    23ac:	f94023e0 	ldr	x0, [sp, #64]
    23b0:	f100001f 	cmp	x0, #0x0
    23b4:	540000ea 	b.ge	23d0 <getNum+0x140>  // b.tcont
        gnFail(fname, "negative value not allowed", arg, name);
    23b8:	f9400be3 	ldr	x3, [sp, #16]
    23bc:	f94013e2 	ldr	x2, [sp, #32]
    23c0:	90000000 	adrp	x0, 2000 <usageErr+0x10>
    23c4:	912f4001 	add	x1, x0, #0xbd0
    23c8:	f94017e0 	ldr	x0, [sp, #40]
    23cc:	97ffff7f 	bl	21c8 <gnFail>

    if ((flags & GN_GT_0) && res <= 0)
    23d0:	b9401fe0 	ldr	w0, [sp, #28]
    23d4:	121f0000 	and	w0, w0, #0x2
    23d8:	7100001f 	cmp	w0, #0x0
    23dc:	54000140 	b.eq	2404 <getNum+0x174>  // b.none
    23e0:	f94023e0 	ldr	x0, [sp, #64]
    23e4:	f100001f 	cmp	x0, #0x0
    23e8:	540000ec 	b.gt	2404 <getNum+0x174>
        gnFail(fname, "value must be > 0", arg, name);
    23ec:	f9400be3 	ldr	x3, [sp, #16]
    23f0:	f94013e2 	ldr	x2, [sp, #32]
    23f4:	90000000 	adrp	x0, 2000 <usageErr+0x10>
    23f8:	912fc001 	add	x1, x0, #0xbf0
    23fc:	f94017e0 	ldr	x0, [sp, #40]
    2400:	97ffff72 	bl	21c8 <gnFail>

    return res;
    2404:	f94023e0 	ldr	x0, [sp, #64]
}
    2408:	a8c57bfd 	ldp	x29, x30, [sp], #80
    240c:	d65f03c0 	ret

0000000000002410 <getLong>:
/* Convert a numeric command-line argument string to a long integer. See the
   comments for getNum() for a description of the arguments to this function. */

long
getLong(const char *arg, int flags, const char *name)
{
    2410:	a9bd7bfd 	stp	x29, x30, [sp, #-48]!
    2414:	910003fd 	mov	x29, sp
    2418:	f90017e0 	str	x0, [sp, #40]
    241c:	b90027e1 	str	w1, [sp, #36]
    2420:	f9000fe2 	str	x2, [sp, #24]
    return getNum("getLong", arg, flags, name);
    2424:	f9400fe3 	ldr	x3, [sp, #24]
    2428:	b94027e2 	ldr	w2, [sp, #36]
    242c:	f94017e1 	ldr	x1, [sp, #40]
    2430:	90000000 	adrp	x0, 2000 <usageErr+0x10>
    2434:	91302000 	add	x0, x0, #0xc08
    2438:	97ffff96 	bl	2290 <getNum>
}
    243c:	a8c37bfd 	ldp	x29, x30, [sp], #48
    2440:	d65f03c0 	ret

0000000000002444 <getInt>:
/* Convert a numeric command-line argument string to an integer. See the
   comments for getNum() for a description of the arguments to this function. */

int
getInt(const char *arg, int flags, const char *name)
{
    2444:	a9bc7bfd 	stp	x29, x30, [sp, #-64]!
    2448:	910003fd 	mov	x29, sp
    244c:	f90017e0 	str	x0, [sp, #40]
    2450:	b90027e1 	str	w1, [sp, #36]
    2454:	f9000fe2 	str	x2, [sp, #24]
    long res;

    res = getNum("getInt", arg, flags, name);
    2458:	f9400fe3 	ldr	x3, [sp, #24]
    245c:	b94027e2 	ldr	w2, [sp, #36]
    2460:	f94017e1 	ldr	x1, [sp, #40]
    2464:	90000000 	adrp	x0, 2000 <usageErr+0x10>
    2468:	91304000 	add	x0, x0, #0xc10
    246c:	97ffff89 	bl	2290 <getNum>
    2470:	f9001fe0 	str	x0, [sp, #56]

    if (res > INT_MAX || res < INT_MIN)
    2474:	f9401fe1 	ldr	x1, [sp, #56]
    2478:	b2407be0 	mov	x0, #0x7fffffff            	// #2147483647
    247c:	eb00003f 	cmp	x1, x0
    2480:	540000ac 	b.gt	2494 <getInt+0x50>
    2484:	f9401fe1 	ldr	x1, [sp, #56]
    2488:	b26183e0 	mov	x0, #0xffffffff80000000    	// #-2147483648
    248c:	eb00003f 	cmp	x1, x0
    2490:	5400010a 	b.ge	24b0 <getInt+0x6c>  // b.tcont
        gnFail("getInt", "integer out of range", arg, name);
    2494:	f9400fe3 	ldr	x3, [sp, #24]
    2498:	f94017e2 	ldr	x2, [sp, #40]
    249c:	90000000 	adrp	x0, 2000 <usageErr+0x10>
    24a0:	91306001 	add	x1, x0, #0xc18
    24a4:	90000000 	adrp	x0, 2000 <usageErr+0x10>
    24a8:	91304000 	add	x0, x0, #0xc10
    24ac:	97ffff47 	bl	21c8 <gnFail>

    return res;
    24b0:	f9401fe0 	ldr	x0, [sp, #56]
}
    24b4:	a8c47bfd 	ldp	x29, x30, [sp], #64
    24b8:	d65f03c0 	ret

Disassembly of section .fini:

00000000000024bc <_fini>:
    24bc:	d503201f 	nop
    24c0:	a9bf7bfd 	stp	x29, x30, [sp, #-16]!
    24c4:	910003fd 	mov	x29, sp
    24c8:	a8c17bfd 	ldp	x29, x30, [sp], #16
    24cc:	d65f03c0 	ret
