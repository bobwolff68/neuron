#ifndef OOLUA_CONSTRUCTORS_H_
#	define  OOLUA_CONSTRUCTORS_H_

#	include "oolua"

class DefaultConstructor
{
public:
	DefaultConstructor(){}
};

class OneParamConstructor
{
public:
	OneParamConstructor(int){}
};


class OneAndTwoParamConstructor
{
public:
	OneAndTwoParamConstructor(int){}
	OneAndTwoParamConstructor(int,int){}
};

class OneAndTwoAndThreeParamConstructor
{
public:
	OneAndTwoAndThreeParamConstructor(int){}
	OneAndTwoAndThreeParamConstructor(int,int){}
	OneAndTwoAndThreeParamConstructor(int,int,int){}
};


OOLUA_CLASS_NO_BASES(DefaultConstructor)
	OOLUA_NO_TYPEDEFS
	OOLUA_ONLY_DEFAULT_CONSTRUCTOR
OOLUA_CLASS_END

OOLUA_CLASS_NO_BASES(OneParamConstructor)
	OOLUA_TYPEDEFS No_default_constructor OOLUA_END_TYPES
	OOLUA_CONSTRUCTORS_BEGIN
		OOLUA_CONSTRUCTOR_1(int )
	OOLUA_CONSTRUCTORS_END
OOLUA_CLASS_END

OOLUA_CLASS_NO_BASES(OneAndTwoParamConstructor)
	OOLUA_TYPEDEFS No_default_constructor OOLUA_END_TYPES
	OOLUA_CONSTRUCTORS_BEGIN
		OOLUA_CONSTRUCTOR_1(int )
		OOLUA_CONSTRUCTOR_2(int,int )
	OOLUA_CONSTRUCTORS_END
OOLUA_CLASS_END

OOLUA_CLASS_NO_BASES(OneAndTwoParamConstructor)
	OOLUA_TYPEDEFS No_default_constructor OOLUA_END_TYPES
	OOLUA_CONSTRUCTORS_BEGIN
		OOLUA_CONSTRUCTOR_1(int )
		OOLUA_CONSTRUCTOR_2(int,int )
	OOLUA_CONSTRUCTORS_END
OOLUA_CLASS_END


#endif
