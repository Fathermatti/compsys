
.pos    0
movq    stack, %rsp            // Set up the stack pointer.
movq    %rsp, %rbp   
movq    $8, %rbx             // Global immediate 8, which is the indexoffset
movq    $0, %rcx
movq    %rcx, %rdi
movq    %rcx, %rsi            
movq    vectorLength(%rcx), %rdx          
mulq    %rbx, %rsi           // Convert indexes to byte-offset indexes
mulq    %rbx, %rdx          
subq    %rbx, %rdx               
call    quicksort
hlt


// Register use:
// - Non-volatile registers: %rdi, %rsi
// - Volatile registers:  %r9, %r10, %r11 %rdx, %rcx, %rax
partition: 
	pushq 	%rdi
	pushq 	%rsi
	movq    sortVector(%rsi), %rdi // int pivot = arr[low];
	movq    %rsi, %r10     // int i = (low - 1);
	subq    %rbx, %r10
	movq    %rdx, %r11    // int j = high + 1;
	addq    %rbx, %r11

fst_do_while:
	addq    %rbx, %r10
	movq	sortVector(%r10), %rcx
	cmpq	%rdi, %rcx
	jl     	fst_do_while
snd_do_while:
	subq    %rbx, %r11
	movq	sortVector(%r11), %r9
	cmpq	%rdi, %r9
	jg		snd_do_while
test_out:
	cmpq	%r10, %r11
	jle		out_of_partition
swap: 
	movq	%r9, sortVector(%r10)
	movq	%rcx, sortVector(%r11)
	jmp 	fst_do_while
out_of_partition:
	movq 	%r11, %rax
	popq	%rsi
	popq	%rdi
	ret
	

// Register use:
// - Non-volatile registers: %rdi, %rsi
// - Volatile registers:  %rdx, %r8, %rax 
// - Stack pointer and base pointer: %rsp, %rbp, 
quicksort:
	cmpq    %rdx, %rsi 
	jge     quicksort_after
part_and_sort: 
	pushq	%rbp      //Sets up stack frame 
	movq	%rsp, %rbp
	movq    $16, %r8
	subq	%r8, %rsp
	movq	%rdx, -8(%rbp)	//Load volatile argument unto stack
	call    partition
	movq    %rax, -16(%rbp)	//Store partioning index on stack

	movq	-16(%rbp), %rdx
	call    quicksort

	movq	-16(%rbp), %rsi
	addq	%rbx, %rsi
	movq	-8(%rbp), %rdx
	call    quicksort

	movq    $16, %r8
	addq    %r8, %rsp
	popq    %rbp
quicksort_after:
	ret

	.pos 0x800
stack:
vectorLength:
	.quad size
sortVector:
	.rand 123 size 1

