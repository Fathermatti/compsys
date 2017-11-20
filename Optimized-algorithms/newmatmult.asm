

//
// void multiply(int dim, int mat1[dim][dim], int mat2[dim][dim], int out[dim][dim])
// {
//     int i, j, k;                                     <--- %rbx, %rbp, %r12
//
//     for (i = 0; i < dim; i++)                        <--- l1:
//     {
//         for (j = 0; j < dim; j++)                    <--- l2:
//         {
//         var acc1 = 0;
//         var acc2 = 0;
//
//             for (k = 0; k < dim-1; k += 2)                <--- l3:
//             {
//                m11 = mat1[i][k]
//                m12 = Mat1[i][k+1]
//                m21 = mat2[k][j]
//                m22 = mat2[k+1][j]
//                mulq m11, m21
//                mulq m12, m22 
//                acc1 += m21
//                acc2 += m22
//             }
//         acc1 += acc2;
//         if (k < dim)
//         {
//             acc1 += mat[i][k] * mat2[k][j];
//         }
//         out[i][j] = acc1;
//         }
//     }
// }
//

    .pos 0
    movq stack, %rsp            // Set up the stack pointer.
    movq $1, %r13               // %r13 = 1
    movq $0, %r14               // %r14 = 0
    movq $8, %r15               // %r15 = 8
    movq dim(%r14), %rax        // Load dimension into RAX.

    movq %rax, %rdi             // Argument 1 is size n of the matrices
    movq mat1, %rsi             // Argument 2 is the address of the first matrice
    movq mat2, %rdx             // Argument 3 is the address of the second matrice
    movq out, %rcx              // Argument 4 is the address of the output matrice
    call multiply
    hlt


multiply: 
    movq    $16, %r8
    subq    %r8, %rsp
    movq    %rax, %r9
    movq    %r9, 0(%rsp)  
    subq    %r13, %r9
    movq    %r9, 8(%rsp)  
    movq $0, %rbx               // i = 0
l1:                             // for(i;...) {
    movq $0, %rbp               //     j = 0 
l2:                             //     for(j,...) {
    movq $0, %r8                //         acc1 = 0
    movq $0, %r9                //         acc2 = 0
    movq $0, %r12               //         k = 0 
    jmp loop_test
l3:                             //         for(k,...) {
    movq %rbx, %r10
    mulq %rax, %r10
    addq %r12, %r10
    mulq %r15, %r10
    movq mat1(%r10), %r11       //             %r11 = mat1[i][k]
    addq %r15, %r10
    movq mat1(%r10), %r13       //             %r13 = mat[i][k+1]

    movq %r12, %r14
    mulq %rax, %r14
    addq %rbp, %r14
    mulq %r15, %r14
    movq mat2(%r14), %rcx       //             %rcx = mat2[k][j]

    mulq %r15, %rax             //             %rax = 8*dim
    addq %rax, %r14
    movq mat2(%r14), %rdx       //             %rdx = mat2[k+1][j]
    movq 0(%rsp), %rax          //             %rax = dim

    mulq %r11, %rcx             //             %rcx = mat1[i][k] * mat2[k][j]
    mulq %r13, %rdx             //             %rdx = mat1[i][k+1] * mat2[k+1][j]

    addq %rcx, %r8              //             acc1 += %rcx
    addq %rdx, %r9              //             acc2 += %rdx

    movq $2, %r10
    addq %r10, %r12             //             k += 2
    
loop_test:
    movq 8(%rsp), %r11          //             %r11 = dim-1
    cmpq %r11, %r12
    jl l3                      //         }

    addq %r8, %r9               //        acc1 += acc2

    cmpq %rax, %r12
    je end_of_loop              //        if(k < dim) {

    movq %rbx, %r10
    mulq %rax, %r10
    addq %r12, %r10
    mulq %r15, %r10
    movq mat1(%r10), %r11       //             %r11 = mat1[i][k]

    movq %r12, %r14
    mulq %rax, %r14
    addq %rbp, %r14
    mulq %r15, %r14
    movq mat2(%r14), %rcx       //             %rcx = mat2[k][j]

    mulq %rcx, %r11             //             %r11 = mat1[i][k] * mat2[k][j]
    addq %r11, %r9              //             acc1 += %r11

end_of_loop:                    //         }
    movq %rbx, %r10
    mulq %rax, %r10
    addq %rbp, %r10
    mulq %r15, %r10
    movq %r9, out(%r10)         //         out[i][j] = acc1

    movq $1, %r8
    addq %r8, %rbp              //         j++
    cmpq %rbp, %rax 
    jne l2                      //     }

    //
    addq %r8, %rbx              //     i++
    cmpq %rbx, %rax
    jne l1                      // }

    movq    $16, %r8
    addq    %r8, %rsp
    ret

    .pos 0x600
stack:
dim:
    .quad size

mat1:
    .rand 123 arrayLength

mat2:
    .rand 123 arrayLength

out:
