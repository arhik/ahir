#include <assert.h>
#include <BitVectors.h>
#include <string.h>
#include <inttypes.h>
#include <Pipes.h>

#define __nbytes(x) ((x % 8 == 0) ? x/8 : (x/8) + 1)
// ---------------  local functions --------------------------------------
uint64_t unpack_bit_vector_into_uint64(uint8_t signed_flag, bit_vector* bv)
{
	uint64_t word = 0;
	uint8_t neg_flag = 0;
	if(signed_flag && __sign_bit(bv)) // negative number.
	{
		neg_flag = 1;
	}

	if(neg_flag) 
		word = -1;	 
	else 
		word = 0;

	uint8_t* word_ptr = (uint8_t*) &word;
        uint32_t asize = __array_size(bv);
	if(asize > 0)
	{
		int i;
		for (i=0; i < 8; i++)
		{
			if(i >= asize)
				break;
			if(neg_flag)
				word_ptr[i] = word_ptr[i] & __get_byte(bv,i);
			else
				word_ptr[i] = word_ptr[i] | __get_byte(bv,i);
		}
	}
	return(word);
}


void  pack_uint64_into_bit_vector(uint8_t signed_flag, uint64_t word, bit_vector* bv)
{
	uint8_t def_byte = 0; 
	uint8_t neg_flag = 0;
	if(signed_flag && (word >> 63))
	{
		neg_flag = 1;
	}
	if (neg_flag) 
		def_byte = 0xff; 
	int i;
	uint8_t* word_ptr = (uint8_t*) &word;
	for (i=0; i < 8; i++)  // uint64_t
	{
		if(i >= __array_size(bv))
			 break;
		uint8_t nb = word_ptr[i];
		if(neg_flag)
			__set_byte(bv,i,(nb & def_byte));
		else
			__set_byte(bv,i,(nb | def_byte));

		// clear the undefined bits.
		__set_undefined_byte(bv,i,0);
	}
	bit_vector_clear_unused_bits(bv);
}


uint64_t truncate_uint64(uint64_t val, uint32_t bit_width)
{
	uint64_t Z = 0;
	uint64_t trunc_mask = ~Z;
	if(bit_width < 64)
		trunc_mask = (~ (trunc_mask << bit_width));

	uint64_t ret_val = val & trunc_mask;
	return(ret_val);
}

void allocate_sized_u8_array(sized_u8_array* a, uint32_t sz)
{
	a->byte_array = calloc(1, sz*sizeof(uint8_t));
	a->undefined_byte_array = calloc(1, sz*sizeof(uint8_t));

	int I;
	for(I = 0; I < sz; I++)
	{
		a->undefined_byte_array[I] = 0xff;
	}

	a->array_size = sz;

}

void free_sized_u8_array(sized_u8_array* a)
{
	cfree (a->byte_array);
	cfree (a->undefined_byte_array);
}


void init_bit_vector(bit_vector* t, uint32_t width)
{
	allocate_sized_u8_array(&(t->val), __nbytes(width));;
	t->width = width;
}

void init_static_bit_vector(bit_vector* t, uint32_t width)
{
	if(t->width == 0)
	{
		allocate_sized_u8_array(&(t->val), __nbytes(width));;
		t->width = width;
	}
}

void free_bit_vector(bit_vector* t)
{
	free_sized_u8_array(&(t->val));
}

void print_bit_vector(bit_vector* t, FILE* ofile)
{
	int I;
	for(I=t->width-1; I>=0; I--)
	{
		if(bit_vector_get_undefined_bit(t,I))
			fprintf(ofile,"?", bit_vector_get_bit(t,I));
		else
			fprintf(ofile,"%u", bit_vector_get_bit(t,I));
	}
}

void printf_bit_vector(bit_vector* t)
{
	print_bit_vector(t,stderr);
}

char to_string_buffer[4096];
char* to_string(bit_vector* t)
{
	int QUAD = 4;
	int I;

	sprintf(to_string_buffer,"");

	for(I=t->width-1; I>=0; I--)
	{
		char buf[16];
		QUAD--;
		if(QUAD == 0)
		{
			sprintf(buf," ");
			QUAD = 4;
		}
		
		if(bit_vector_get_undefined_bit(t,I))
			sprintf(buf,"?");
		else
			sprintf(buf,"%u", bit_vector_get_bit(t,I));

		strcat(to_string_buffer, buf);
	}
	return(to_string_buffer);
}


uint8_t is_undefined_bit(bit_vector* t, uint32_t index)
{
	return(bit_vector_get_undefined_bit(t,index));
}

uint8_t has_undefined_bit(bit_vector* t)
{
	int I;
	for(I=t->width-1; I>=0; I--)
	{
		if(bit_vector_get_undefined_bit(t,I))
			return(1);
	}
	return(0);
}

// -----------------   utility functions.
uint32_t  __array_size(bit_vector* x)
{
	return(x->val.array_size);
} 
void      __set_byte(bit_vector* x, uint32_t byte_index, uint8_t v)
{
	if(byte_index < __array_size(x))
		x->val.byte_array[byte_index] = v;
}

uint8_t   __get_byte(bit_vector* x, uint32_t byte_index)
{
	if(byte_index < __array_size(x))
		return(x->val.byte_array[byte_index]);
	else
		return(0);
}
void      __set_undefined_byte(bit_vector* x, uint32_t byte_index, uint8_t v)
{
	if(byte_index < __array_size(x))
		x->val.undefined_byte_array[byte_index] = v;
}

uint8_t   __get_undefined_byte(bit_vector* x, uint32_t byte_index)
{
	if(byte_index < __array_size(x))
		return(x->val.undefined_byte_array[byte_index]);
	else
		return(0);
}

uint8_t   __sign_bit(bit_vector* x) 
{
	//
	// the location of the  most-significant byte.
	//
	uint32_t byte_position = (x->width-1)/8;


	//
	// position of the sign-bit in the
	// MSByte.
	//
	uint32_t offset = (x->width - 1) % 8;
	uint8_t bit_pos_mask = (0x1 << offset);

	// check the bit.
	if(__get_byte(x,byte_position) & bit_pos_mask)
		return(1);
	else
		return(0);
}

// ----------------   test functions.  --------------------
// return 1 if all bits are cleared.
uint8_t bit_vector_is_zero(bit_vector* t)
{
	uint8_t ret_val = 1;
	uint32_t asize = __array_size(t);
	uint8_t  cW = 0;
	int i;
	for(i = 0; i < asize; i++)
	{
		cW += 8;
		uint8_t valid_bit_count = ((cW <= t->width)  ? 8 :  (cW % t->width));
		uint8_t bmask =  (0xff >> (8 - valid_bit_count));
		uint8_t b = (bmask & __get_byte(t,i));
		if(b != 0)
		{
			ret_val = 0;
			break;
		}	
	}
	return(ret_val);
}

// ----------------   initialization, conversions to C types.  --------------------
void      bit_vector_clear_unused_bits(bit_vector* t)
{
	uint32_t aSize = __array_size(t);
	uint32_t W8 = 8*aSize;
	if(W8 == t->width)
		return;

	uint8_t invalid_bits_in_top_byte = (W8 % t->width);
	uint8_t bmask = (0xff >> invalid_bits_in_top_byte);
	uint8_t tbyte = (bmask & __get_byte(t, aSize-1));
	__set_byte(t, aSize-1, tbyte);
}

uint64_t  bit_vector_to_uint64(uint8_t signed_flag, bit_vector* t)
{
	uint64_t rv = 0;
	rv = unpack_bit_vector_into_uint64(signed_flag,t);


	// truncate high order bits if non-negative.
	if(!signed_flag || __sign_bit(t))
	{
		rv = truncate_uint64(rv, t->width);
	}

	return(rv);
}

float     bit_vector_to_float(uint8_t signed_flag, bit_vector* t)
{
  float rv = 0;
  if(signed_flag)
    {
      int64_t v = bit_vector_to_uint64(signed_flag, t);
      rv =  v;
    }
  else
    {
      uint64_t v = bit_vector_to_uint64(signed_flag, t);
      rv =  v;
    }
}

double   bit_vector_to_double(uint8_t signed_flag, bit_vector* t)
{
  double rv = 0;
  if(signed_flag)
    {
      int64_t v = bit_vector_to_uint64(signed_flag, t);
      rv =  v;
    }
  else
    {
      uint64_t v = bit_vector_to_uint64(signed_flag, t);
      rv =  v;
    }
}


// useful assignments..
void bit_vector_assign_uint64(uint8_t signed_flag, bit_vector* dest, uint64_t src)
{
	uint64_cast_to_bit_vector(signed_flag, dest, &src);
}


void bit_vector_assign_float(uint8_t signed_flag, bit_vector* dest, float src)
{
	float_cast_to_bit_vector(signed_flag, dest, &src);
}

void bit_vector_assign_double(uint8_t signed_flag, bit_vector* dest, double src)
{
	double_cast_to_bit_vector(signed_flag, dest, &src);
}

void bit_vector_assign_string(bit_vector* dest, char* init_string)
{
	bit_vector_clear(dest);
	if(strlen(init_string) == dest->width)
	{
		int I, fI;
		for(I = 0, fI = dest->width; I < fI; I++)
		{
			bit_vector_set_bit(dest, (dest->width-I)-1, (init_string[I] == '1'));
		}
	}
	else
	{
		fprintf(stderr,"Error: init string %s not of correct size (%d).\n", init_string, dest->width);
	}
}

// ----------   casts, bit-casts ---------------------------------------------------------
void bit_vector_cast_to_bit_vector(uint8_t signed_flag, bit_vector* dest, bit_vector* src)
{
	uint32_t min_width = __min(src->width,dest->width);
	uint32_t J;
	uint8_t neg_flag = 0;
        uint8_t undef_flag = 0;

	bit_vector_clear(dest);

	for(J=0; J < min_width; J++)
	{
	  uint8_t tb = bit_vector_get_bit(src,J);
	  uint8_t u_tb = bit_vector_get_undefined_bit(src,J);
	  if(u_tb)
		undef_flag = 0xff;
	
	  bit_vector_set_bit(dest,J, tb);
	  bit_vector_set_undefined_bit(dest,J, undef_flag);
	}

	if(signed_flag && __sign_bit(src))
	  {
	    // sign-extend in dest.
	    int I;
	    for(I = src->width; I <= dest->width; I++)
	      {
		bit_vector_set_bit(dest, I, 1);
	  	bit_vector_set_undefined_bit(dest,I, undef_flag);
	      }
	  }
}

void bit_vector_bitcast_to_bit_vector(bit_vector* dest, bit_vector* src)
{
	bit_vector_cast_to_bit_vector(0, dest, src);
}


void bit_vector_cast_to_uint64(uint8_t signed_flag, uint64_t* dest, bit_vector* src)
{
	*dest = bit_vector_to_uint64(signed_flag, src);
}

void bit_vector_bitcast_to_uint64( uint64_t* dest, bit_vector* src)
{
	*dest = bit_vector_to_uint64(0, src);
}

void uint64_cast_to_bit_vector(uint8_t signed_flag, bit_vector* dest, uint64_t* src)
{
	pack_uint64_into_bit_vector(signed_flag, *src, dest);
}

void uint64_bitcast_to_bit_vector(bit_vector* dest, uint64_t* src)
{
	uint64_cast_to_bit_vector(0, dest,src);
}

void bit_vector_cast_to_float(uint8_t signed_flag, float* dest, bit_vector* src)
{
	*dest = bit_vector_to_float(signed_flag, src);
}

void bit_vector_bitcast_to_float( float* dest, bit_vector* src)
{
	uint32_t v = bit_vector_to_uint64(0,src);
	*dest = *((float*) &v);
}

void float_cast_to_bit_vector(uint8_t signed_flag,  bit_vector* dest, float* src)
{
  if(signed_flag)
    {
      int64_t v = *src;
      pack_uint64_into_bit_vector(signed_flag, v, dest);
    }
  else
    {
      uint64_t v = *src;
      pack_uint64_into_bit_vector(signed_flag, v, dest);
    }
}

void float_bitcast_to_bit_vector( bit_vector* dest, float* src)
{
	uint64_t v = (uint64_t)  (*((uint32_t*) src));
        pack_uint64_into_bit_vector(0, v, dest );
}

void bit_vector_cast_to_double(uint8_t signed_flag, double* dest, bit_vector* src)
{
	*dest = bit_vector_to_double(signed_flag, src);
}

void bit_vector_bitcast_to_double( double* dest, bit_vector* src)
{
	uint64_t v = bit_vector_to_uint64(0,src);
	*dest = *((double*) &v);
}

void double_cast_to_bit_vector(uint8_t signed_flag, bit_vector* dest, double *src)
{
 if(signed_flag)
    {
      int64_t v = *src;
      pack_uint64_into_bit_vector(signed_flag, v, dest);
    }
  else
    {
      uint64_t v = *src;
      pack_uint64_into_bit_vector(signed_flag, v, dest );
    }
}

void double_bitcast_to_bit_vector(bit_vector* dest, double *src)
{
	uint64_t v =  *((uint64_t*) src);
        pack_uint64_into_bit_vector(0, v, dest );
}

///   set clear.

void bit_vector_clear(bit_vector* s)
{
	uint32_t asize = __array_size(s);
	uint32_t i;
	for(i = 0; i < asize; i++)
	{
		__set_byte(s,i,0);
		__set_undefined_byte(s,i,0);
	}
}

void bit_vector_set(bit_vector* s)
{
	uint32_t asize = __array_size(s);
	uint32_t i;
	for(i = 0; i < asize; i++)
	{
		__set_byte(s,i,0xff);
		__set_undefined_byte(s,i,0);
	}
}

// ------------------            bit-wise operations
void bit_vector_or(bit_vector* r, bit_vector* s, bit_vector* t)
{
	assert(r->width == s->width); assert(s->width == t->width);
	int i;
	uint32_t asize = __array_size(r);
	for(i = 0; i < asize; i++)
	{
		uint8_t rb = __get_byte(r,i);
		uint8_t urb = __get_undefined_byte(r,i);
		uint8_t sb = __get_byte(s,i);
		uint8_t usb = __get_undefined_byte(s,i);
		__set_byte(t,i, (rb | sb));
		__set_undefined_byte(t,i,((urb & usb) | (urb & (~sb)) | (usb & (~rb))));
	}
	bit_vector_clear_unused_bits(t);
}

void bit_vector_nor(bit_vector* r, bit_vector* s, bit_vector* t)
{
	assert(r->width == s->width); assert(s->width == t->width);
	int i;
	uint32_t asize = __array_size(r);
	for(i = 0; i < asize; i++)
	{
		uint8_t rb = __get_byte(r,i);
		uint8_t urb = __get_undefined_byte(r,i);
		uint8_t sb = __get_byte(s,i);
		uint8_t usb = __get_undefined_byte(s,i);
		__set_byte(t,i, (~(rb | sb)));
		__set_undefined_byte(t,i,((urb & usb) | (urb & (~sb)) | (usb & (~rb))));
	}
	bit_vector_clear_unused_bits(t);
}
void bit_vector_and(bit_vector* r, bit_vector* s, bit_vector* t)
{
	assert(r->width == s->width); assert(s->width == t->width);
	int i;
	uint32_t asize = __array_size(r);
	for(i = 0; i < asize; i++)
	{
		uint8_t rb = __get_byte(r,i);
		uint8_t urb = __get_undefined_byte(r,i);
		uint8_t sb = __get_byte(s,i);
		uint8_t usb = __get_undefined_byte(s,i);
		__set_byte(t,i, (rb & sb));
		__set_undefined_byte(t,i,((urb & usb) | (urb & sb) | (usb & rb)));
	}
	bit_vector_clear_unused_bits(t);
}
void bit_vector_nand(bit_vector* r, bit_vector* s, bit_vector* t)
{
	assert(r->width == s->width); assert(s->width == t->width);
	int i;
	uint32_t asize = __array_size(r);
	for(i = 0; i < asize; i++)
	{
		uint8_t rb = __get_byte(r,i);
		uint8_t urb = __get_undefined_byte(r,i);
		uint8_t sb = __get_byte(s,i);
		uint8_t usb = __get_undefined_byte(s,i);
		__set_byte(t,i, ~(rb & sb));
		__set_undefined_byte(t,i,((urb & usb) | (urb & sb) | (usb & rb)));
	}
	bit_vector_clear_unused_bits(t);
}
void bit_vector_xor(bit_vector* r, bit_vector* s, bit_vector* t)
{
	assert(r->width == s->width); assert(s->width == t->width);
	int i;
	uint32_t asize = __array_size(r);
	for(i = 0; i < asize; i++)
	{
		__set_byte(t,i, (__get_byte(r,i) ^ __get_byte(s,i)));
		__set_undefined_byte(t,i,__get_undefined_byte(r,i) | __get_undefined_byte(s,i));
	}
	bit_vector_clear_unused_bits(t);
}
void bit_vector_xnor(bit_vector* r, bit_vector* s, bit_vector* t)
{
	assert(r->width == s->width); assert(s->width == t->width);
	int i;
	uint32_t asize = __array_size(r);
	for(i = 0; i < asize; i++)
	{
		__set_byte(t,i, (~(__get_byte(r,i) ^ __get_byte(s,i))));
		__set_undefined_byte(t,i,__get_undefined_byte(r,i) | __get_undefined_byte(s,i));
	}
	bit_vector_clear_unused_bits(t);
}

void  bit_vector_not(bit_vector* src, bit_vector* dest)
{
	assert(__array_size(src) == __array_size(dest));
	uint32_t J;
	uint32_t nbytes = __array_size(src);
	for(J=0; J < nbytes; J++)
	{
		__set_byte(dest,J,~(__get_byte(src,J)));
		__set_undefined_byte(dest,J,__get_undefined_byte(src,J));
	}
	bit_vector_clear_unused_bits(dest);
}


// -----------------------------------  arithmetic ---------------------
// ++
void bit_vector_increment(bit_vector* s)
{
	uint16_t curr_carry = 1;
	int i;
	for(i = 0; i < __array_size(s); i++)
	{
		uint16_t op =  __get_byte(s,i);
		uint16_t result = op + curr_carry;
		if(result & 0xff00)
			curr_carry = 1;
		else
			curr_carry = 0;
		__set_byte(s,i,result);
	}
	bit_vector_clear_unused_bits(s);
}

void bit_vector_plus(bit_vector* r, bit_vector* s, bit_vector* t)
{
	assert(__array_size(r) == __array_size(s));
	assert(__array_size(t) == __array_size(s));
	uint32_t asize = __array_size(r);
	uint16_t curr_carry = 0;
	
	int i;
	for(i = 0; i < asize; i++)
	{
		uint16_t op1 =  __get_byte(r,i);
		uint16_t op2 =  __get_byte(s,i);
		uint16_t result = (op1 + op2) + curr_carry;
		if(result & 0xff00)
			curr_carry = 1;
		else
			curr_carry = 0;

		__set_byte(t,i,(result & 0xff));
		__set_undefined_byte(t,i,__get_undefined_byte(r,i) | __get_undefined_byte(s,i));
	}
	bit_vector_clear_unused_bits(t);
}

void bit_vector_minus(bit_vector* r, bit_vector* s, bit_vector* t)
{
	assert(__array_size(r) == __array_size(s));
	assert(__array_size(t) == __array_size(s));
	bit_vector_not(s,t);
	bit_vector_plus(r,t,t);
	bit_vector_increment(t);
	bit_vector_clear_unused_bits(t);
}

// support this only for widths up to 64.
void bit_vector_mul(bit_vector* r, bit_vector* s, bit_vector* t)
{
	assert(__array_size(r) == __array_size(s));
	assert(__array_size(t) == __array_size(s));

	uint64_t op1,op2;
	op1 = bit_vector_to_uint64(0,r);
	op2 = bit_vector_to_uint64(0,s);

	uint64_t result = op1*op2;
	bit_vector_assign_uint64(0,t,result);
}

//
// support this only for widths up to 64..
//
void bit_vector_div(bit_vector* r, bit_vector* s, bit_vector* t)
{
	assert(__array_size(r) == __array_size(s));
	assert(__array_size(t) == __array_size(s));

	uint64_t op1,op2;
	op1 = bit_vector_to_uint64(0,r);
	op2 = bit_vector_to_uint64(0,s);


	uint64_t result = 0;
	if(op2 != 0)
		result = op1/op2;
	else
	{
		fprintf(stderr,"Error: divide by 0... % " PRId64 "/%" PRId64 "for bit-width %d.\n", op1,op2,r->width);
	}

	pack_uint64_into_bit_vector(0,result,t);
}

void bit_vector_set_bit(bit_vector* f, uint32_t bp, uint8_t bv)
{
	if (bp < f->width)
	{
		uint64_t byte_id = bp/8;
		uint64_t byte_mask = (bv ? (((uint64_t) 0x1) <<  (bp%8)) : ~(((uint64_t) 0x1) << (bp%8)));
		if(bv)
			__set_byte(f,byte_id, (__get_byte(f,byte_id) | byte_mask));
		else
			__set_byte(f,byte_id, (__get_byte(f,byte_id) & byte_mask));
	}
}
void bit_vector_set_undefined_bit(bit_vector* f, uint32_t bp, uint8_t bv)
{
	if (bp < f->width)
	{
		uint64_t byte_id = bp/8;
		uint64_t byte_mask = (bv ? (((uint64_t) 0x1) <<  (bp%8)) : ~(((uint64_t) 0x1) << (bp%8)));
		if(bv)
			__set_undefined_byte(f,byte_id, (__get_undefined_byte(f,byte_id) | byte_mask));
		else
			__set_undefined_byte(f,byte_id, (__get_undefined_byte(f,byte_id) & byte_mask));
	}
}
uint8_t bit_vector_get_bit(bit_vector* f, uint32_t bp)
{
	uint8_t ret_val = 0;
	if (bp < f->width)
	{
		uint64_t byte_id = bp/8;
		uint64_t byte_mask = (((uint64_t) 0x1) <<  (bp%8));
		if(__get_byte(f,byte_id) & byte_mask)
			ret_val = 1;
	}
	return(ret_val);
}

uint8_t bit_vector_get_undefined_bit(bit_vector* f, uint32_t bp)
{
	uint8_t ret_val = 0;
	if (bp < f->width)
	{
		uint64_t byte_id = bp/8;
		uint64_t byte_mask = (((uint64_t) 0x1) <<  (bp%8));
		if(__get_undefined_byte(f,byte_id) & byte_mask)
			ret_val = 1;
	}
	return(ret_val);
}

void bit_vector_bitsel(bit_vector* f, bit_vector* s, bit_vector* result)
{
	bit_vector_clear(result);

	uint64_t sv;
	sv = bit_vector_to_uint64(0,s);

	bit_vector_set_bit(result, 0, bit_vector_get_bit(f,sv));
	bit_vector_set_undefined_bit(result, 0, bit_vector_get_undefined_bit(f,sv));
}

// the next two are a bit inefficient..
void bit_vector_concatenate(bit_vector* f, bit_vector* s, bit_vector* result)
{
	assert(result->width == (f->width + s->width));
	bit_vector_clear(result);
	int I = 0;
	int J;
	for(J = 0; J < s->width; J++)
	{
		bit_vector_set_bit(result,J,bit_vector_get_bit(s,J));	
		bit_vector_set_undefined_bit(result,J,bit_vector_get_undefined_bit(s,J));	
	}
	for(J = 0; J < f->width; J++)
	{
		bit_vector_set_bit(result,J + s->width,bit_vector_get_bit(f,J));	
		bit_vector_set_undefined_bit(result,J + s->width,bit_vector_get_undefined_bit(f,J));	
	}
}

// slice src starting from low_index upwards to fill dest.
void bit_vector_slice(bit_vector* src, bit_vector* dest, uint32_t low_index)
{
	// src should have enough width..  
	// src->width-1 >= low_index + dest->width  - 1.
	assert(src->width >= low_index + dest->width);
	bit_vector_clear(dest);

	int J;
	uint32_t high_index =  low_index + dest->width - 1;
	for(J = low_index; J <= high_index; J++)
	{
		bit_vector_set_bit(dest, J-low_index, bit_vector_get_bit(src,J));
		bit_vector_set_undefined_bit(dest, J-low_index, bit_vector_get_undefined_bit(src,J));
	}
}

// insert src into dest starting at low_index.
void bit_vector_insert(bit_vector* src, bit_vector* dest, uint32_t low_index)
{
	assert(dest->width >= low_index + src->width);
	bit_vector_clear(dest);

	int J;
	for(J = 0; J < src->width; J++)
	{
		bit_vector_set_bit(dest, J+low_index, bit_vector_get_bit(src,J));
		bit_vector_set_undefined_bit(dest, J+low_index, bit_vector_get_undefined_bit(src,J));
	}
}

// -----------------------   shifts and rotates. -------------------------------
void bit_vector_shift_right(uint8_t signed_flag, bit_vector* r, bit_vector* s, bit_vector* t)
{
	// clear it.
	bit_vector_clear(t);


	uint8_t sign_val = 0;
	if(signed_flag)
	{
		if(__sign_bit(r))
		{
			sign_val = 1;
			bit_vector_not(t,t);
		}
	}

	uint32_t shift_amount = bit_vector_to_uint64(0,s);
	if(shift_amount < t->width)
	{
		int I;
		for(I=shift_amount; I < t->width; I++)
		{
			bit_vector_set_bit(t,I-shift_amount, bit_vector_get_bit(r,I));
			bit_vector_set_undefined_bit(t,I-shift_amount, bit_vector_get_undefined_bit(r,I));
		}
	}

}
void bit_vector_shift_left(bit_vector* r, bit_vector* s, bit_vector* t)
{
	bit_vector_clear(t);
	uint32_t shift_amount = bit_vector_to_uint64(0,s);

	if(shift_amount < t->width)
	{
		int I;
		for(I=0; I < (r->width - shift_amount); I++)
		{
			bit_vector_set_bit(t,I+shift_amount, bit_vector_get_bit(r,I));
			bit_vector_set_undefined_bit(t,I+shift_amount, bit_vector_get_undefined_bit(r,I));
		}
	}
}
void bit_vector_rotate_left(bit_vector* r, bit_vector* s, bit_vector* t)
{
	assert(r->width == t->width);
	bit_vector_clear(t);
	uint32_t rotate_amount = bit_vector_to_uint64(0,s);
	uint32_t word_size = r->width;

	int I;
	for(I=0; I < word_size; I++)
	{
		bit_vector_set_bit(t,((I+rotate_amount)%word_size), bit_vector_get_bit(r,I));
		bit_vector_set_undefined_bit(t,((I+rotate_amount)%word_size), bit_vector_get_undefined_bit(r,I));
	}
}
void bit_vector_rotate_right(bit_vector* r, bit_vector* s, bit_vector* t)
{
	assert(r->width == t->width);
	bit_vector_clear(t);
	uint32_t rotate_amount = bit_vector_to_uint64(0,s);
	uint32_t word_size = r->width;

	int I;
	for(I=rotate_amount; I < (word_size + rotate_amount); I++)
	{
		bit_vector_set_bit(t,((I-rotate_amount)%word_size), bit_vector_get_bit(r,(I%word_size)));
		bit_vector_set_undefined_bit(t,((I-rotate_amount)%word_size), bit_vector_get_undefined_bit(r,(I%word_size)));
	}
}

// --------------------    comparisons -------------------------------------
// returns 0 if equal, 1 if r > s, 2 if r < s.
uint8_t uint64_compare(uint8_t signed_flag, uint64_t a, uint64_t b, uint64_t width)
{
	if(width > 64)
		width = 64;
	
	// signs.
	uint8_t sa = (signed_flag ? ((a & (((uint64_t) 0x1) << width-1)) != 0) : 0);
	uint8_t sb = (signed_flag ? ((b & (((uint64_t) 0x1) << width-1)) != 0) : 0);

	if(sa && !sb)
		return(IS_LESS);
	else if(!sa && sb)
		return(IS_GREATER);

	// sign-extend.
	// 
	uint64_t sign_extend_mask = (((int64_t ) -1)  << width);
	if(sa)
	{
		a = a | sign_extend_mask;
	}
	if(sb)
	{
		b = b | sign_extend_mask;
	}


	if(a == b)
		return(IS_EQUAL);
	else if(a < b)
	{
		if(sa && sb)
			return(IS_GREATER);
		else
			return(IS_LESS);
	}
	else
	{
		if(sa && sb)
			return(IS_LESS);
		else
			return(IS_GREATER);
	}
}

// returns 0 if equal, 1 if r > s, 2 if r < s.
uint8_t bit_vector_compare(uint8_t signed_flag, bit_vector* r, bit_vector* s)
{
	uint8_t undef_flag = 0;

	// aggressive undef handling.
	if(has_undefined_bit(r) || has_undefined_bit(s))
		return(IS_UNDEFINED);

	// raw comparison only between bit-vectors of 
	// equal length.
	assert(r->width == s->width);

	// raw comparison.
	int i;
	uint8_t re = 1;
	uint8_t rg = 0;
	uint8_t rl = 0;

	uint8_t sr = (signed_flag ? __sign_bit(r) : 0);
	uint8_t ss = (signed_flag ? __sign_bit(s) : 0);

	if(sr && !ss)
	{ // r is negative and s is positive
		return(IS_LESS);
	}
	else if(!sr && ss)
	{// r is positive, s is negative.
		return(IS_GREATER);
	}
	
	
	// both signs are the same.
	for(i = r->width-1; i >= 0; i--)
	{
		uint8_t rb = bit_vector_get_bit(r,i); 
		uint8_t sb = bit_vector_get_bit(s,i); 
		if(re && (rb && !sb))
		{
			re = 0;
			rg = 1;
			break;
		}
		else if(re && (!rb && sb))
		{
			re = 0;
			rl = 1;
			break;
		}
	}


	if(sr && ss)
	{ // both negative.. swap rl, rg.
		uint8_t tmp = rg;
		rg = rl;
		rl = rg;
	}

	if(re)
		return(IS_EQUAL);
	else if(rg) 
		return(IS_GREATER);
	else
		return(IS_LESS);
}

void bit_vector_equal(uint8_t signed_flag, bit_vector* r, bit_vector* s, bit_vector* t)
{
	bit_vector_clear(t);
	if(bit_vector_compare(signed_flag,r,s) == IS_EQUAL)
	{
		bit_vector_set_bit(t,0,1);
	}
	
		
}
void bit_vector_not_equal(uint8_t signed_flag, bit_vector* r, bit_vector* s, bit_vector* t)
{
	bit_vector_clear(t);
	if(!bit_vector_compare(signed_flag,r,s) == IS_EQUAL)
	{
		bit_vector_set_bit(t,0,1);
	}
}
void bit_vector_less(uint8_t signed_flag, bit_vector* r, bit_vector* s, bit_vector* t)
{
	bit_vector_clear(t);
	uint8_t bvc = bit_vector_compare(signed_flag,r,s);
	if(bvc == IS_LESS)
	{
		bit_vector_set_bit(t,0,1);
	}
	else if(bvc == IS_UNDEFINED)
	{
		bit_vector_set_undefined_bit(t,0,1);
	}
}
void bit_vector_less_equal(uint8_t signed_flag, bit_vector* r, bit_vector* s, bit_vector* t)
{
	bit_vector_clear(t);
	uint8_t cmp_result = bit_vector_compare(signed_flag,r,s);
	if((cmp_result == IS_LESS) || (cmp_result == IS_EQUAL))
	{
		bit_vector_set_bit(t,0,1);
	}
	else if(cmp_result == IS_UNDEFINED)
	{
		bit_vector_set_undefined_bit(t,0,1);
	}
}
void bit_vector_greater(uint8_t signed_flag, bit_vector* r, bit_vector* s, bit_vector* t)
{
	bit_vector_clear(t);
	uint8_t cmp_result = bit_vector_compare(signed_flag,r,s);
	if(cmp_result == IS_GREATER)
	{
		bit_vector_set_bit(t,0,1);
	}
	else if(cmp_result == IS_UNDEFINED)
	{
		bit_vector_set_undefined_bit(t,0,1);
	}
}
void bit_vector_greater_equal(uint8_t signed_flag, bit_vector* r, bit_vector* s, bit_vector* t)
{
	bit_vector_clear(t);
	uint8_t cmp_result = bit_vector_compare(signed_flag,r,s);
	if((cmp_result == IS_GREATER) || (cmp_result == IS_EQUAL))
	{
		bit_vector_set_bit(t,0,1);
	}
	else if(cmp_result == IS_UNDEFINED)
	{
		bit_vector_set_undefined_bit(t,0,1);
	}
}


uint8_t isNormalFp32(float a)
{
	float tA = a;
	uint32_t ua = 	 *((uint32_t*) &tA);
	uint32_t sign_ua = (ua & (1 << 31));
	uint32_t exp_ua  = (ua & (0xff << 23));

	uint8_t is_normal = ((exp_ua != 0) && (~exp_ua != 0));
	return(is_normal);	
}

uint8_t isNormalFp64(float a)
{
	float tA = a;
	uint64_t u64_1 = 1;
	uint64_t u64_0x7ff = 0x7ff;

	uint64_t ua = 	 *((uint64_t*) &tA);
	uint64_t sign_ua = (ua & (u64_1 << 63));
	uint64_t exp_ua  = (ua & (u64_0x7ff << 51));

	uint8_t is_normal = ((exp_ua != 0) && (~exp_ua != 0));
	return(is_normal);	
}
uint8_t fp32_unordered(float a, float b)
{
	return(!isNormalFp32(a) || !isNormalFp32(b));

}
uint8_t fp64_unordered(float a, float b)
{
	return(!isNormalFp64(a) || !isNormalFp64(b));
}
	

void write_bit_vector_to_pipe(char* pipe_name, bit_vector* bv)
{
	switch(bv->width)
	{
		case 8:
			write_uint8(pipe_name, (uint8_t) bit_vector_to_uint64(0,bv));
			break;
		case 16:
			write_uint16(pipe_name, (uint16_t) bit_vector_to_uint64(0,bv));
			break;
		case 32:
			write_uint32(pipe_name, (uint32_t) bit_vector_to_uint64(0,bv));
			break;
		case 64:
			write_uint64(pipe_name, (uint64_t) bit_vector_to_uint64(0,bv));
			break;
		default:
			write_uint8_n(pipe_name, bv->val.byte_array, bv->val.array_size);	
			break;
	}
}


void read_bit_vector_from_pipe(char* pipe_name, bit_vector* bv)
{
	uint64_t val;
	bit_vector_clear(bv);
	switch(bv->width)
	{
		case 8:
			val = read_uint8(pipe_name);
			bit_vector_assign_uint64(0,bv,val);
			break;
		case 16:
			val = read_uint16(pipe_name);
			bit_vector_assign_uint64(0,bv,val);
			break;
		case 32:
			val = read_uint32(pipe_name);
			bit_vector_assign_uint64(0,bv,val);
			break;
		case 64:
			val = read_uint64(pipe_name);
			bit_vector_assign_uint64(0,bv,val);
			break;
		default:
			read_uint8_n(pipe_name, bv->val.byte_array, bv->val.array_size);	
			break;
	}
}


