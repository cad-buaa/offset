/*******************************************************************/
/*    Copyright (c) 1989-2020 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
/**
* ACIS Compound
*/
#if !defined(INT_ARRAY_HXX)
#define INT_ARRAY_HXX
#include "dcl_compound.h"
#include "acis.hxx"
/**
 * @file int_array.hxx
 * \addtogroup ACISCOMPOUND
 * @{
 */
/**
*<summary>Integer array.</summary>
*/
class DECL_COMPOUND INT_ARRAY : public ACIS_OBJECT
{
	/**
	*<summary>The integer array.</summary>
	*/
	int* _array;
	/**
	*<summary>Array size(populated).</summary>
	*/            
	int _size;
	/**
	*<summary>Array capacity.</summary>
	*/
	int _capacity;
	/**
	*<summary>Minimum capacity, used as incremental factor.</summary>
	*/
	int _minimum_capacity;
	/**
	*<summary>Copy constructor, forbiden.</summary>
	*/
	INT_ARRAY(const INT_ARRAY&);
	/**
	*<summary>Copy operator, forbiden.</summary>
	*/
	INT_ARRAY& operator=(const INT_ARRAY&);
	/**
	*<summary>Resize array.</summary>
	*/
	void resize_array();
	/**
	*<summary>Default construcotr, forbiden.</summary>
	*/
	INT_ARRAY();

public:
	/**
	*<summary>Constructor, specify initial capacity.</summary>
	*<param name="capacity">Minimum capacity</param>
	*/
	INT_ARRAY(const int& capacity);
	/**
	*<summary>Constructor, specify initial array and it size. The object owns the array.</summary>
	*<param name="array">Integer array</param>
	*<param name="size">Array size</param>
	*/
	INT_ARRAY(int*& array, const int& size);
	/**
	*<summary>Destructor.</summary>
	*/     
	~INT_ARRAY();
	/**
	*<summary>Returns the element of the array for the input index.</summary>
	*<param name="index">Index.</param>
	*/
	int&  operator[](int index);
	/**
	*<summary>Returns array size (populated).</summary>
	*/
	int size();
	/**
	*<summary>Sets new capacity. Must be bigger than current capacity. Otherwise, ignored.</summary>
	*<param name="max">Ne capacity.</param>
	*/
	logical set_capacity(const int& max);
	/**
	*<summary>Get capacity.</summary>
	*/
	int get_capacity();
	/**
	*<summary>Add new element to array, if capacity reached, array will grow.</summary>
	*<param name="elem">New aray element.</param>
	*/
	int add(const int& elem);
	/**
	* <summary>Get the array.</summary>
	*/
	const int* array(){return _array;};

};
/** @}*/ 
#endif