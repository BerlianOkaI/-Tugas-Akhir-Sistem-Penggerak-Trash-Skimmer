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

#ifndef FUZZYLOGIC_H_
#define FUZZYLOGIC_H_

#define DISC_SIZE               10

typedef unsigned int u_int;

typedef enum
{
    // Type of membership function for defining Fuzzy Set
    TRP_L,			// Trapezoid Left
    TRP_C,			// Trapezoid Center
    TRP_R,			// Trapezoid Right
    TRI,			// Triangular
	SINGLE			// Singleton
} FS_type;

typedef enum
{
    INPUT,
    OUTPUT
} FrameType;


typedef struct FS_param
{
    // Parameters of Fuzzy Set in which it is characterized by
    float thr1, thr2, thr3, thr4;
    FS_type mu_type;
} FS_param;

typedef struct UnivDisc
{
    // Universe of Discourse. Can be used to determine the domain
    // where linguistic variables reside
    float low_bond, up_bond;
    float interval;
} UnivDisc;

/* PROTOTYPES */
// Membership Function Shape
float triangular(float thr_left, float thr_center, float thr_right, float x);
float trapezoid_left(float thr_left, float thr_right, float x);
float trapezoid_right(float thr_left, float thr_right, float x);
float trapezoid_center(float thr_left1, float thr_left2, float thr_right1, float thr_right2, float x);
float singleton(float thr_center, float x);

// Operator for array only
float minimum(float* array_input, u_int array_size);
float maximum(float* array_input, u_int array_size);

// Other operator
float minimum(float x1, float x2);
float maximum(float x1, float x2);

// Classes
class FuzzySet
{
private:
    FS_param _param;
public:
    // Initialization
	void set_up(FS_type the_type, float thr_1);
    void set_up(FS_type the_type, float thr_1, float thr_2);
    void set_up(FS_type the_type, float thr_1, float thr_2, float thr_3);
    void set_up(FS_type the_type, float thr_1, float thr_2, float thr_3, float thr_4);

    // Membership Function
    float mu_func(int x);
    float mu_func(float x);
};

class FuzzyFrame
{
private:
    FuzzySet* _ling_sets;
    u_int _ling_size;
    UnivDisc _domain;
    FrameType _type;
public:
    void Frame_SetUp(FuzzySet* sets, u_int _ling_size, float x_left, float x_right, FrameType FF_type);
    void domainSetUp(float x_left, float x_right, float interval);
	void Set_SetUp(u_int indx, FS_type the_type, float thr_1);
    void Set_SetUp(u_int indx, FS_type the_type, float thr_1, float thr_2);
    void Set_SetUp(u_int indx, FS_type the_type, float thr_1, float thr_2, float thr_3);
    void Set_SetUp(u_int indx, FS_type the_type, float thr_1, float thr_2, float thr_3, float thr_4);

    float get_muvalue(u_int indx, float x);
    FuzzySet* getFSAddress(void);
    int get_size(void);
    UnivDisc get_domain(void);
};

class FuzzyRule
{
/***
 * [Consist of Combination of antecedent and consequent
 * that can be used to define one of the IF-THEN rules
 * in Fuzzy Logic]
 *
 * // HOW TO USE IT
 *
 *    1.  Declare the FuzzyRule objects as an array (for example, FuzzyRule myRule[10])
 *        "this means that we create Fuzzy System with 10 IF-THEN rule"
 *
 *    2.  Declare 2 array of FuzzyFrame, antecedent one and consequent one, where the
 *        size of each array is correspond with number of linguistic variable for antecedent
 *        and consequent. for example, if we have a IF-THEN rule like:
 *        "IF Temperature is Cold & Hummidity is Dry THEN HeaterPower is HIGH"
 *        then, there are 2 linguistic variable for antecedent and 1 linguistic variable
 *        for consequent. Thus, we have to declare antecedent FuzzyFrame array with size of 2
 *        and consequent FuzzyFrame array with size of 1.
 *        Note: for any number of FuzzyRule
 *        that is created, we just have to declare 2 array only.
 *
 *    3.  Declare 2 array of unsigned int, antecedent one and consequent one, where the size
 *        and index of array is correspond with the number and the index of linguistic
 *        variable in antecedent and consequent. For above example, in antecedent 1st element
 *        (0th index) of antecedent's FuzzyFrame array is a FuzzyFrame for linguistic variable
 *        "Temperature" while the second element is "Humidity". The int value that is passed
 *        in each element is correspond with the index of linguistic value for each linguistic variable.
 *        For example, if Temperature has 2 linguistic value, 1st is Cold and 2nd is Hot, and Humidity
 *        has 3 linguistic value, 1st is Very Dry; 2nd is Dry; and 3rd is Wet, then the int array for antecedent
 *        to define IF-THEN Rule above is [0, 1] (Temp is cold (0) and Hum is dry (1)). This is also
 *        applies for the consequent
 *
 *    4.  Do Rule_SetUp method with passing arguments with FuzzyFrames and array of int that we
 *        have declare before
***/
private:
    FuzzyFrame* _antecedent_frames;
    FuzzyFrame* _consequent_frames;
    u_int* _antecedent_rules;
    u_int* _consequent_rules;

    u_int _input_frame_size;
    u_int _output_frame_size;

public:
    void Rule_SetUp(FuzzyFrame* input_frames, u_int* input_rules, u_int FR_input_size, FuzzyFrame* output_frames, u_int* output_rules, u_int FR_output_size);
    float Evaluate(float* input, float output, u_int output_id);
    UnivDisc get_output_domain(u_int output_id);
};

class FuzzySystem
{
private:
    FuzzyRule* _rules;
    u_int _total_rules;
public:
    FuzzySystem(FuzzyRule* Rules, u_int total_rules);
    float Evaluate(float* input, float output, u_int output_id);
    float Defuzzyfication(float* input, u_int output_id);
};


#endif // FUZZYLOGIC_H
