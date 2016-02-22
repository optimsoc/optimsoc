
optimsoc_inc_dir ${LISNOC_RTL}/dma

lisnoc_add_file dma/lisnoc_dma.v
lisnoc_add_file dma/lisnoc_dma_target.v
lisnoc_add_file dma/lisnoc_dma_initiator.v
lisnoc_add_file dma/lisnoc_dma_initiator_wbreq.v
lisnoc_add_file dma/lisnoc_dma_initiator_nocreq.v
lisnoc_add_file dma/lisnoc_dma_initiator_nocresp.v
lisnoc_add_file dma/lisnoc_dma_request_table.v
lisnoc_add_file dma/lisnoc_dma_wbinterface.v

lisnoc_add_file lisnoc_arb_rr.v