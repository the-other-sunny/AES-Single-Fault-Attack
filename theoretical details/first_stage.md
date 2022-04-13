# First stage reduction

The last round key is noted $K$:

$$
    K =
    \begin{pmatrix}
        K_{1} & K_{5} & K_{ 9} & K_{13}\\
        K_{2} & K_{6} & K_{10} & K_{14}\\
        K_{3} & K_{7} & K_{11} & K_{15}\\
        K_{4} & K_{8} & K_{12} & K_{16}
    \end{pmatrix}
$$

The cipher text is noted $Y$:

$$
    Y = 
    \begin{pmatrix}
        Y_{1} & Y_{5} & Y_{ 9} & Y_{13}\\
        Y_{2} & Y_{6} & Y_{10} & Y_{14}\\
        Y_{3} & Y_{7} & Y_{11} & Y_{15}\\
        Y_{4} & Y_{8} & Y_{12} & Y_{16}
    \end{pmatrix}
$$

Reversing the final state $Y$ to 9-th round output $Y^9$ gives:

$$
    Y^9 = 
    \begin{pmatrix}
        \operatorname{S^{-1}}(K_{ 1} + Y_{ 1}) &
        \operatorname{S^{-1}}(K_{ 5} + Y_{ 5}) &
        \operatorname{S^{-1}}(K_{ 9} + Y_{ 9}) &
        \operatorname{S^{-1}}(K_{13} + Y_{13})\\

        \operatorname{S^{-1}}(K_{14} + Y_{14}) &
        \operatorname{S^{-1}}(K_{ 2} + Y_{ 2}) &
        \operatorname{S^{-1}}(K_{ 6} + Y_{ 6}) &
        \operatorname{S^{-1}}(K_{10} + Y_{10})\\

        \operatorname{S^{-1}}(K_{11} + Y_{11}) &
        \operatorname{S^{-1}}(K_{15} + Y_{15}) &
        \operatorname{S^{-1}}(K_{ 3} + Y_{ 3}) &
        \operatorname{S^{-1}}(K_{ 7} + Y_{ 7})\\

        \operatorname{S^{-1}}(K_{ 8} + Y_{ 8}) &
        \operatorname{S^{-1}}(K_{12} + Y_{12}) &
        \operatorname{S^{-1}}(K_{16} + Y_{16}) &
        \operatorname{S^{-1}}(K_{ 4} + Y_{ 4})
    \end{pmatrix}
$$

The fault is injected between 7-th round `MixColumns` and 8-th round `MixColumns`.  
After the following `MixColumns` (8-th round's one), the differential state consist of one non-zero column noted $(A, B, C, D)$.  
Depending on the location of that column (which depends back on the position the fault was injected at), the differential state obtained after the next `MixColumn` is described below.

$$
    \begin{pmatrix}
        A & 0 & 0 & 0\\
        B & 0 & 0 & 0\\
        C & 0 & 0 & 0\\
        D & 0 & 0 & 0
    \end{pmatrix}
    \longrightarrow
    \begin{pmatrix}
        2A &  D &  C & 3B\\
        A &  D & 3C & 2B\\
        A & 3D & 2C &  B\\
        3A & 2D &  C &  B
    \end{pmatrix}
$$

$$
    \begin{pmatrix}
        0 & A & 0 & 0\\
        0 & B & 0 & 0\\
        0 & C & 0 & 0\\
        0 & D & 0 & 0
    \end{pmatrix}
    \longrightarrow
    \begin{pmatrix}
        3B & 2A &  D &  C\\
        2B &  A &  D & 3C\\
         B &  A & 3D & 2C\\
         B & 3A & 2D &  C
    \end{pmatrix}
$$

$$
    \begin{pmatrix}
        0 & 0 & A & 0\\
        0 & 0 & B & 0\\
        0 & 0 & C & 0\\
        0 & 0 & D & 0
    \end{pmatrix}
    \longrightarrow
    \begin{pmatrix}
         C & 3B & 2A &  D\\
        3C & 2B &  A &  D\\
        2C &  B &  A & 3D\\
         C &  B & 3A & 2D
    \end{pmatrix}
$$

$$
    \begin{pmatrix}
        0 & 0 & 0 & A\\
        0 & 0 & 0 & B\\
        0 & 0 & 0 & C\\
        0 & 0 & 0 & D
    \end{pmatrix}
    \longrightarrow
    \begin{pmatrix}
     D &  C & 3B & 2A\\
     D & 3C & 2B &  A\\
    3D & 2C &  B &  A\\
    2D &  C &  B & 3A
    \end{pmatrix}
$$

Equating the adequate differential state from above ones to $Y^9 \oplus Y'^9$ gives the equations used for the first stage reduction.
