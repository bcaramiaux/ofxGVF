
#ifndef	MAXMIX
#define MAXMIX


#define Word_setInt(p, v) ((p)->w_long = (v))
#define Word_setFloat(p, v) ((p)->w_float = (v))
#define Word_setSymbol(p, v) ((p)->w_sym = (v))
#define Word_setPointer(p, v) ((p)->w_obj = (struct object *)(v))

#define Word_getInt(p) ((p)->w_long)
#define Word_getFloat(p) ((p)->w_float)
#define Word_getSymbol(p) ((p)->w_sym)
#define Word_getPointer(p) ((void *)(p)->w_obj)

#pragma mark -

#define setVoid(p) (Word_setInt(&(p)->a_w, 0), (p)->a_type = A_NOTHING)
#define setInt(p, v) (Word_setInt(&(p)->a_w, (v)), (p)->a_type = A_LONG)
#define setFloat(p, v) (Word_setFloat(&(p)->a_w, (v)), (p)->a_type = A_FLOAT)
#define setSymbol(p, v) (Word_setSymbol(&(p)->a_w, (v)), (p)->a_type = A_SYM)
#define setPointer(p, v) (Word_setPointer(&(p)->a_w, (v)), (p)->a_type = A_OBJ)

#define isVoid(p) ((p)->a_type == A_NOTHING)
#define isFloat(p) ((p)->a_type == A_FLOAT) 
#define isInt(p) ((p)->a_type == A_LONG) 
#define isNumber(p) ((p)->a_type == A_LONG || (p)->a_type == A_FLOAT) 
#define isSymbol(p) ((p)->a_type == A_SYM)
#define isPointer(p) ((p)->a_type == A_OBJ)

#define getInt(p) Word_getInt(&(p)->a_w)
#define getFloat(p) Word_getFloat(&(p)->a_w)
#define getNumberInt(p) (isInt(p) ? getInt(p) : (int)getFloat(p))
#define getNumberFloat(p) (isFloat(p) ? getFloat(p) : (double)getInt(p))
#define getSymbol(p) Word_getSymbol(&(p)->a_w)
#define getPointer(p) Word_getPointer(&(p)->a_w)

#define getTypeID(p) ((p)->a_type)


#endif