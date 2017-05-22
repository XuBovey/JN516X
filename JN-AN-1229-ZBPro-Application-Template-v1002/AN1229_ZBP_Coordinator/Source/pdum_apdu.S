    .global s_asAPduPool
    .section    .data.s_asAPduPool,"aw",@progbits
    .align 4
    .type    s_asAPduPool, @object
    .size    s_asAPduPool, 24
s_asAPduPool:
    .global pdum_apduZDP
pdum_apduZDP:
    .long    s_asapduZDPInstances
    .short    0
    .short    100
    .short    3
    .zero     2
    .global pdum_apduMyData
pdum_apduMyData:
    .long    s_asapduMyDataInstances
    .short    0
    .short    16
    .short    1
    .zero     2
