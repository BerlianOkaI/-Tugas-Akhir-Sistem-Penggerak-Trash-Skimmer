/***
  * Author          : Berlian Oka Irvianto  (Indonesia)
  * Last Modified   : Friday, December 15th 2023
  *
  * "In the name of God, the Most Gracious, the Most Merciful"
  *
  * This is a library that consist a tool (FuzzySet, FuzzyFrame, FuzzyRule
  * and FuzzySystem) to implement Fuzzy Logic. This library was originally
  * created by the author for applying Fuzzy Logic as a control system in
  * embedded system (using avr microcontroller like ATmega328p etc). This
  * library also requires the user to play a lot with array declaration;
  * objects like FuzzySet, FuzzyFrame and FuzzyRule should be declared
  * as an array (for example, FuzzySet mySet[3], FuzzyRule myRule[27], etc).
  * For more details on how to use this library, please have a look on
  * program example that were provided by the author in this repository.
  *
  * // HOW TO USE IN GENERAL
  *
  * 1. Declaration of Linguistic Values of each Linguistic Variables as array of class <FuzzySet>
  *    // Example:
  *    ```
  *    FuzzySet Temperatures[3]         // for linguistic variable temperature, it has 3 linguistic values (Cold, Cool, Hot)
  *    ```
  *
  * 2. Declaration of Linguistic Variables as array of class <FuzzyFrame>
  *    // Example:
  *    ```
  *    FuzzyFrame Antecedent[2]         // there are 2 frame as input (antecedent) that will be evaluated in FuzzyRule
  *    ```
  *
  * 3. Declaration of Fuzzy Rule as array of class <FuzzyRule> and declaration of antecedent and consequent
  *    as 2D unsigned int array
  *    // Example:
  *    ```
  *    FuzzyRule myRules[27]            // There are 27 rules. The number of rules depend on number of combination
  *                                     // of linguistic values of each linguistic variables in antecedent
  *
  *    unsigned int input_rule[27][2]   // Of course there are 27 antecedent. Each antecedent consist of 2 linguistic values
  *                                     // of different linguistic variables to define specific "IF" part in IF-THEN rule
  *
  *    unsigned int output_rule[27][1]  // Of course there are 27 consequent. Each consequent consist of 1 linguistic values
  *                                     // of 1 linguistic variables to define specific "THEN" part in IF-THEN rule
  *    ```
  *
  * 4. Declaration of FuzzySystem
  *    // Example:
  *    ```
  *    FuzzySystem mySystem(myRules, 27)// Declare a system that consist of 27 rule and use FuzzyRule that has been declared as argument
  *
  * 5. Setting up the FuzzySets, FuzzyFrames, and FuzzyRules that has been declared. This consist
  *    of determining the membership function of each FuzzySet(s), Determining wich FuzzyFrames as
  *    input frames (antecedent) and other one (consequent), Determining IF-THEN rule for each rule
  *    etc. For more details, please have a look on example program that were provided in this repository
  *
  * 6. Use the FuzzySystem to produce output (crisp) given by inputs
  *
***/

#include "FuzzyLogic.h"

/* MEMBERSHIP FUNCTION SHAPE */
//
float triangular(float thr_left, float thr_center, float thr_right, float x)
{
    /*
    This function is one of simplest membership function that
    defines a Fuzzy Set.

    Its shape is a triangular with three threshold (points) that create
    a shape of triangles. thr_center parameter is argument in which the value
    of membership function maximum (1.0)
    */
    if (x <= thr_left)
        // Absolutely Zero
        {return 0.0;}
    else if(x > thr_left && x <= thr_center)
        // Rising slope
        {return (x - thr_left)/(thr_center - thr_left);}
    else if(x > thr_center && x <= thr_right)
        // Falling slope
        {return (thr_right - x)/(thr_right - thr_center);}
    else
        // Absolutely Zero
        {return 0.0;}
}

float trapezoid_left(float thr_left, float thr_right, float x)
{
    /*
    This function is one of simplest membership function that
    defines a Fuzzy Set.

    Its shape is a trapezoidal with two threshold (points) that create
    a shape of trapezoid in which the value of uFunc will be one for x -> -infty
    and the value of uFunc will be zero for x -> +infty.
    */
    if(x <= thr_left)
        // x is certain belong to the set
        {return 1.0;}
    else if(x > thr_left && x <= thr_right)
        // Falling slope
        {return (thr_right - x)/(thr_right - thr_left);}
    else
        // Absolute zero
        {return 0.0;}
}

float trapezoid_right(float thr_left, float thr_right, float x)
{
    /*
    This function is one of simplest membership function that
    defines a Fuzzy Set.

    Its shape is a trapezoidal with two threshold (points) that create
    a shape of trapezoid in which the value of uFunc will be one for x -> +infty
    and the value of uFunc will be zero for x -> -infty.
    */
    if(x <= thr_left)
        // Absolute zero
        {return 0.0;}
    else if(x > thr_left && x <= thr_right)
        // Rising Slope
        {return (x - thr_left)/(thr_right - thr_left);}
    else
        // x is certain belong to the set
        {return 1.0;}
}

float trapezoid_center(float thr_left1, float thr_left2, float thr_right1, float thr_right2, float x)
{
    /*
    This function is one of simplest membership function that
    defines a Fuzzy Set.

    Its shape is a trapezoidal with two threshold (points) that create
    a shape of trapezoid in which the value of uFunc will be one for x between thr_left2 and thr_right1.
    Its shape will be a trapesium
    */
    if (x <= thr_left1)
        // Absolutely Zero
        {return 0.0;}
    else if(x > thr_left1 && x <= thr_left2)
        // Rising slope
        {return (x - thr_left1)/(thr_left2 - thr_left1);}
    else if(x > thr_left2 && x <= thr_right1)
        // x is certainly belong to the set
        {return 1.0;}
    else if(x > thr_right1 && x <= thr_right2)
        // Falling slope
        {return (thr_right2 - x)/(thr_right2 - thr_right1);}
    else
        // Absolutely Zero
        {return 0.0;}
}

float singleton(float thr_center, float x)
{
	/*
    This function is one of simplest membership function that
    defines a Fuzzy Set.

	It define a Fuzzy Set with membership function that is unity at
	a single particular point (thr_center) on the universe of discourse and
	zero in elsewhere
    */
	if (thr_center == x)
		{return 1.0;}
	else 
		{return 0.0;}
}



/* ARRAY OPERATOR */
//
float minimum(float* array_input, u_int array_size)
{
    /* Use it to find minimum from array of
     * membership degree only
    */
    float result;
    result = 1.0;
    for (u_int i=0; i<array_size; i++)
    {
        if (result > array_input[i]){result = array_input[i];}
    }
    return result;
}

float maximum(float* array_input, u_int array_size)
{
    /* Use it to find maximum from array of
     * membership degree only
    */
    float result;
    result = 0.0;
    for (u_int i=0; i<array_size; i++)
    {
        if (result < array_input[i]){result = array_input[i];}
    }
    return result;
}

/* OTHER OPERATOR */
//
float minimum(float x1, float x2)
{
    // Simple algorithm to find minimum value of two number
    if (x1 < x2)    {return x1;}
    else            {return x2;}
}
float maximum(float x1, float x2)
{
    // Simple algorithm to find minimum value of two number
    if (x1 > x2)    {return x1;}
    else            {return x2;}
}

/* CLASSES */
//
void FuzzySet::set_up(FS_type the_type, float thr_1)
{
	_param.mu_type = the_type;
	_param.thr1 = thr_1;
}

void FuzzySet::set_up(FS_type the_type, float thr_1, float thr_2)
{
    _param.mu_type = the_type;
    _param.thr1 = thr_1;
    _param.thr2 = thr_2;
}

void FuzzySet::set_up(FS_type the_type, float thr_1, float thr_2, float thr_3)
{
    _param.mu_type = the_type;
    _param.thr1 = thr_1;
    _param.thr2 = thr_2;
    _param.thr3 = thr_3;
}
void FuzzySet::set_up(FS_type the_type, float thr_1, float thr_2, float thr_3, float thr_4)
{
    _param.mu_type = the_type;
    _param.thr1 = thr_1;
    _param.thr2 = thr_2;
    _param.thr3 = thr_3;
    _param.thr4 = thr_4;
}

float FuzzySet::mu_func(int x)
{
    // Calculate degree of membership
    float _val = (float)x;
    switch (this->_param.mu_type)
    {
    case TRP_L:
        _val = trapezoid_left(this->_param.thr1, this->_param.thr2, _val);
        break;
    case TRP_C:
        _val = trapezoid_center(this->_param.thr1, this->_param.thr2, this->_param.thr3, this->_param.thr4, _val);
        break;
    case TRP_R:
        _val = trapezoid_right(this->_param.thr1, this->_param.thr2, _val);
        break;
    case TRI:
        _val = triangular(this->_param.thr1, this->_param.thr2, this->_param.thr3, _val);
        break;
	case SINGLE:
		_val = singleton(this->_param.thr1, _val);
		break;
    default:
        _val = 0.0;
    }
    return _val;
}

float FuzzySet::mu_func(float x)
{
    // Calculate degree of membership
    float _val = x;
    switch (this->_param.mu_type)
    {
    case TRP_L:
        _val = trapezoid_left(this->_param.thr1, this->_param.thr2, _val);
        break;
    case TRP_C:
        _val = trapezoid_center(this->_param.thr1, this->_param.thr2, this->_param.thr3, this->_param.thr4, _val);
        break;
    case TRP_R:
        _val = trapezoid_right(this->_param.thr1, this->_param.thr2, _val);
        break;
    case TRI:
        _val = triangular(this->_param.thr1, this->_param.thr2, this->_param.thr3, _val);
        break;
	case SINGLE:
		_val = singleton(this->_param.thr1, _val);
		break;
    default:
        _val = 0.0;
    }
    return _val;
}

void FuzzyFrame::Frame_SetUp(FuzzySet* sets, u_int _ling_size, float x_left, float x_right, FrameType FF_type)
{
    this->_domain.low_bond = x_left;
    this->_domain.up_bond = x_right;
    this->_domain.interval = (x_right - x_left)/(1.0*(DISC_SIZE - 1));
    this->_ling_sets = sets;
    this->_ling_size = _ling_size;
}
void FuzzyFrame::domainSetUp(float x_left, float x_right, float interval)
{
    this->_domain.low_bond = x_left;
    this->_domain.up_bond = x_right;
    this->_domain.interval = interval;
}
void FuzzyFrame::Set_SetUp(u_int indx, FS_type the_type, float thr_1)
{
	this->_ling_sets[indx].set_up(the_type, thr_1);
}
void FuzzyFrame::Set_SetUp(u_int indx, FS_type the_type, float thr_1, float thr_2)
{
    this->_ling_sets[indx].set_up(the_type, thr_1, thr_2);
}
void FuzzyFrame::Set_SetUp(u_int indx, FS_type the_type, float thr_1, float thr_2, float thr_3)
{
    this->_ling_sets[indx].set_up(the_type, thr_1, thr_2, thr_3);
}
void FuzzyFrame::Set_SetUp(u_int indx, FS_type the_type, float thr_1, float thr_2, float thr_3, float thr_4)
{
    this->_ling_sets[indx].set_up(the_type, thr_1, thr_2, thr_3, thr_4);
}

float FuzzyFrame::get_muvalue(u_int indx, float x)
{
    return _ling_sets[indx].mu_func(x);
}

FuzzySet* FuzzyFrame::getFSAddress(void)
{
    return this->_ling_sets;
}

int FuzzyFrame::get_size(void)
{
    return this->_ling_size;
}
UnivDisc FuzzyFrame::get_domain(void)
{
    return this->_domain;
}


void FuzzyRule::Rule_SetUp(FuzzyFrame* input_frames, u_int* input_rules, u_int FR_input_size, FuzzyFrame* output_frames, u_int* output_rules, u_int FR_output_size)
{
    this->_antecedent_frames = input_frames;
    this->_consequent_frames = output_frames;
    this->_antecedent_rules = input_rules;
    this->_consequent_rules = output_rules;
    this->_input_frame_size = FR_input_size;
    this->_output_frame_size = FR_output_size;
}

float FuzzyRule::Evaluate(float* input, float output, u_int output_id)
{
    // Determine the degree of fulfillment
    float alpha = 1.0;
    float dummy;
    for (u_int atc=0; atc < this->_input_frame_size; atc++)
    {
        dummy = this->_antecedent_frames[atc].get_muvalue(this->_antecedent_rules[atc], input[atc]);
        alpha = minimum(dummy, alpha);
    }
    // Get minimum value between alpha and consequent mu_value of output
    dummy = this->_consequent_frames[output_id].get_muvalue(this->_consequent_rules[output_id], output);
    // save the result
    return minimum(alpha, dummy);
}
UnivDisc FuzzyRule::get_output_domain(u_int output_id)
{
    return this->_consequent_frames[0].get_domain();
}


FuzzySystem::FuzzySystem(FuzzyRule* Rules, u_int total_rules)
{
    this->_rules = Rules;
    this->_total_rules = total_rules;
}

float FuzzySystem::Evaluate(float* input_, float output_, u_int output_id_)
{
    // Agregatting fuzzy output (degree of membership of output) over all rules
    float result = 0.0;
    float dummy;
    for (u_int rule_id=0; rule_id < this->_total_rules; rule_id++)
    {
        dummy = this->_rules[rule_id].Evaluate(input_, output_, output_id_);
        result = maximum(result, dummy);
    }
    // in here, we have found maximum output fuzzy from all of our rules
    return result;
}

float FuzzySystem::Defuzzyfication(float* input, u_int output_id)
{
    // Finding crisp output of fuzzy output
    // via centroid methods (weight is degree of membership)
    float weight = 0;
    float weight_avg = 0;
    float mu_;
    UnivDisc evaluated_domain = this->_rules[0].get_output_domain(output_id);

    for (float y = evaluated_domain.low_bond; y <= evaluated_domain.up_bond; y = y+evaluated_domain.interval)
    {
        mu_ = this->Evaluate(input, y, output_id);
        weight = weight + mu_;
        weight_avg = weight_avg + mu_ * y;
    }
    if (weight == 0) {weight = 1.0;}            // Precaution for weight = 0 (error division by 0)
    return weight_avg / weight;
}
