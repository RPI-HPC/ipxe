#ifndef _IPXE_DMA_H
#define _IPXE_DMA_H

/** @file
 *
 * DMA mappings
 *
 */

FILE_LICENCE ( GPL2_OR_LATER_OR_UBDL );

#include <stdint.h>
#include <ipxe/api.h>
#include <ipxe/io.h>
#include <ipxe/iobuf.h>
#include <ipxe/malloc.h>
#include <config/ioapi.h>

#ifdef DMAAPI_OP
#define DMAAPI_PREFIX_op
#else
#define DMAAPI_PREFIX_op __op_
#endif

#ifdef DMAAPI_FLAT
#define DMAAPI_PREFIX_flat
#else
#define DMAAPI_PREFIX_flat __flat_
#endif

/** A DMA mapping */
struct dma_mapping {
	/** Device-side address */
	physaddr_t addr;
	/** Platform mapping token */
	void *token;
};

/** A DMA-capable device */
struct dma_device {
	/** DMA operations */
	struct dma_operations *op;
	/** Addressable space mask */
	physaddr_t mask;
	/** Total number of mappings (for debugging) */
	unsigned int mapped;
	/** Total number of allocations (for debugging) */
	unsigned int allocated;
};

/** DMA operations */
struct dma_operations {
	/**
	 * Map buffer for DMA
	 *
	 * @v dma		DMA device
	 * @v addr		Buffer address
	 * @v len		Length of buffer
	 * @v flags		Mapping flags
	 * @v map		DMA mapping to fill in
	 * @ret rc		Return status code
	 */
	int ( * map ) ( struct dma_device *dma, physaddr_t addr, size_t len,
			int flags, struct dma_mapping *map );
	/**
	 * Unmap buffer
	 *
	 * @v dma		DMA device
	 * @v map		DMA mapping
	 */
	void ( * unmap ) ( struct dma_device *dma, struct dma_mapping *map );
	/**
	 * Allocate and map DMA-coherent buffer
	 *
	 * @v dma		DMA device
	 * @v len		Length of buffer
	 * @v align		Physical alignment
	 * @v map		DMA mapping to fill in
	 * @ret addr		Buffer address, or NULL on error
	 */
	void * ( * alloc ) ( struct dma_device *dma, size_t len, size_t align,
			     struct dma_mapping *map );
	/**
	 * Unmap and free DMA-coherent buffer
	 *
	 * @v dma		DMA device
	 * @v addr		Buffer address
	 * @v len		Length of buffer
	 * @v map		DMA mapping
	 */
	void ( * free ) ( struct dma_device *dma, void *addr, size_t len,
			  struct dma_mapping *map );
	/**
	 * Set addressable space mask
	 *
	 * @v dma		DMA device
	 * @v mask		Addressable space mask
	 */
	void ( * set_mask ) ( struct dma_device *dma, physaddr_t mask );
};

/** Device will read data from host memory */
#define DMA_TX 0x01

/** Device will write data to host memory */
#define DMA_RX 0x02

/** Device will both read data from and write data to host memory */
#define DMA_BI ( DMA_TX | DMA_RX )

/**
 * Calculate static inline DMA I/O API function name
 *
 * @v _prefix		Subsystem prefix
 * @v _api_func		API function
 * @ret _subsys_func	Subsystem API function
 */
#define DMAAPI_INLINE( _subsys, _api_func ) \
	SINGLE_API_INLINE ( DMAAPI_PREFIX_ ## _subsys, _api_func )

/**
 * Provide a DMA I/O API implementation
 *
 * @v _prefix		Subsystem prefix
 * @v _api_func		API function
 * @v _func		Implementing function
 */
#define PROVIDE_DMAAPI( _subsys, _api_func, _func ) \
	PROVIDE_SINGLE_API ( DMAAPI_PREFIX_ ## _subsys, _api_func, _func )

/**
 * Provide a static inline DMA I/O API implementation
 *
 * @v _prefix		Subsystem prefix
 * @v _api_func		API function
 */
#define PROVIDE_DMAAPI_INLINE( _subsys, _api_func ) \
	PROVIDE_SINGLE_API_INLINE ( DMAAPI_PREFIX_ ## _subsys, _api_func )

/**
 * Map buffer for DMA
 *
 * @v dma		DMA device
 * @v addr		Buffer address
 * @v len		Length of buffer
 * @v flags		Mapping flags
 * @v map		DMA mapping to fill in
 * @ret rc		Return status code
 */
static inline __always_inline int
DMAAPI_INLINE ( flat, dma_map ) ( struct dma_device *dma, physaddr_t addr,
				  size_t len __unused, int flags __unused,
				  struct dma_mapping *map ) {

	/* Use physical address as device address */
	map->addr = addr;

	/* Increment mapping count (for debugging) */
	if ( DBG_LOG )
		dma->mapped++;

	return 0;
}

/**
 * Unmap buffer
 *
 * @v dma		DMA device
 * @v map		DMA mapping
 */
static inline __always_inline void
DMAAPI_INLINE ( flat, dma_unmap ) ( struct dma_device *dma,
				    struct dma_mapping *map __unused ) {

	/* Decrement mapping count (for debugging) */
	if ( DBG_LOG )
		dma->mapped--;
}

/**
 * Allocate and map DMA-coherent buffer
 *
 * @v dma		DMA device
 * @v len		Length of buffer
 * @v align		Physical alignment
 * @v map		DMA mapping to fill in
 * @ret addr		Buffer address, or NULL on error
 */
static inline __always_inline void *
DMAAPI_INLINE ( flat, dma_alloc ) ( struct dma_device *dma, size_t len,
				    size_t align, struct dma_mapping *map ) {
	void *addr;

	/* Allocate buffer */
	addr = malloc_phys ( len, align );
	map->addr = virt_to_phys ( addr );

	/* Increment allocation count (for debugging) */
	if ( DBG_LOG && addr )
		dma->allocated++;

	return addr;
}

/**
 * Unmap and free DMA-coherent buffer
 *
 * @v dma		DMA device
 * @v addr		Buffer address
 * @v len		Length of buffer
 * @v map		DMA mapping
 */
static inline __always_inline void
DMAAPI_INLINE ( flat, dma_free ) ( struct dma_device *dma,
				   void *addr, size_t len,
				   struct dma_mapping *map __unused ) {

	/* Free buffer */
	free_phys ( addr, len );

	/* Decrement allocation count (for debugging) */
	if ( DBG_LOG )
		dma->allocated--;
}

/**
 * Set addressable space mask
 *
 * @v dma		DMA device
 * @v mask		Addressable space mask
 */
static inline __always_inline void
DMAAPI_INLINE ( flat, dma_set_mask ) ( struct dma_device *dma __unused,
				       physaddr_t mask __unused ) {

	/* Nothing to do */
}

/**
 * Map buffer for DMA
 *
 * @v dma		DMA device
 * @v addr		Buffer address
 * @v len		Length of buffer
 * @v flags		Mapping flags
 * @v map		DMA mapping to fill in
 * @ret rc		Return status code
 */
int dma_map ( struct dma_device *dma, physaddr_t addr, size_t len,
	      int flags, struct dma_mapping *map );

/**
 * Unmap buffer
 *
 * @v dma		DMA device
 * @v map		DMA mapping
 */
void dma_unmap ( struct dma_device *dma, struct dma_mapping *map );

/**
 * Allocate and map DMA-coherent buffer
 *
 * @v dma		DMA device
 * @v len		Length of buffer
 * @v align		Physical alignment
 * @v map		DMA mapping to fill in
 * @ret addr		Buffer address, or NULL on error
 */
void * dma_alloc ( struct dma_device *dma, size_t len, size_t align,
		   struct dma_mapping *map );

/**
 * Unmap and free DMA-coherent buffer
 *
 * @v dma		DMA device
 * @v addr		Buffer address
 * @v len		Length of buffer
 * @v map		DMA mapping
 */
void dma_free ( struct dma_device *dma, void *addr, size_t len,
		struct dma_mapping *map );

/**
 * Set addressable space mask
 *
 * @v dma		DMA device
 * @v mask		Addressable space mask
 */
void dma_set_mask ( struct dma_device *dma, physaddr_t mask );

/**
 * Initialise DMA device
 *
 * @v dma		DMA device
 * @v op		DMA operations
 */
static inline __always_inline void dma_init ( struct dma_device *dma,
					      struct dma_operations *op ) {

	/* Set operations table */
	dma->op = op;
}

/**
 * Set 64-bit addressable space mask
 *
 * @v dma		DMA device
 */
static inline __always_inline void
dma_set_mask_64bit ( struct dma_device *dma ) {

	/* Set mask to maximum physical address */
	dma_set_mask ( dma, ~( ( physaddr_t ) 0 ) );
}

/**
 * Map I/O buffer for transmitting data to device
 *
 * @v dma		DMA device
 * @v iobuf		I/O buffer
 * @v map		DMA mapping to fill in
 * @ret rc		Return status code
 */
static inline __always_inline int
dma_map_tx_iob ( struct dma_device *dma, struct io_buffer *iobuf,
		 struct dma_mapping *map ) {

	/* Map I/O buffer */
	return dma_map ( dma, virt_to_phys ( iobuf->data ), iob_len ( iobuf ),
			 DMA_TX, map );
}

extern struct io_buffer * dma_alloc_rx_iob ( struct dma_device *dma, size_t len,
					     struct dma_mapping *map );

#endif /* _IPXE_DMA_H */
