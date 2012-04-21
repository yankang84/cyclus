// IsoVector.h
#if !defined(_ISOVECTOR_H)
#define _ISOVECTOR_H

#include <map>
#include <set>
#include <utility>
#include <math.h>
#include <vector>
#include <string>
#include <libxml/tree.h>

#include "Table.h"
#include "UseMatrixLib.h"

/* -- Useful Definitions -- */
/**
   avagadro's number
*/
#define AVOGADRO 6.02e23

/**
   smallest kilogram value
*/
#define EPS_KG 1e-6

/**
   smallest percent
*/
#define EPS_PERCENT 1e-14
/* -- */

/* -- Useful Typedefs -- */
class IsoVector;

/**
   Isotope integer, ZZZAAA
 */
typedef int Iso;

/**
   map of isotope integer to value (mass or atom)
 */
typedef std::map<Iso, double> CompMap;

/* --- Useful Structs --- */
/**
   a container fully describing the isovector's composition
 */
struct composition {
  int ID;
  CompMap* mass_fractions;
  double mass_normalizer;
  double atom_normalizer;
  composition* parent;
  int decay_time;
  composition(CompMap* fracs, double mass_norm, double atom_norm) {
    ID = 0;
    mass_fractions = fracs;
    mass_normalizer = mass_norm;
    atom_normalizer = atom_norm;
    parent = 0;
    decay_time = 0;
  }
  bool logged() {
    return (ID > 0);
  }
  void delete_map() {
    delete mass_fractions;
  }
};
/* --- */

/**
   map of recipe name to composition
 */
typedef std::map<std::string, composition*> RecipeMap; 

/**
   set of decay times
 */
typedef std::set<int> decay_times;

/**
   map of composition times decayed
 */
typedef std::map<composition*, decay_times*> DecayTimesMap; 

/**
   map of decay time to composition
 */
typedef std::map<int, composition*> DaughterMap; 

/**
   map of recipe composition to its decayed daughters
 */
typedef std::map<composition*, DaughterMap*> DecayChainMap; 
/* -- */

/** 
   @class IsoVector 
    
   The IsoVector Class is an intelligent, mass-based isotopic composition 
   container. It reports normalized mass or atomic fractions, as requested.
  
   @section recipes Recipes 
   Recipes are pre-defined IsoVectors. Recipes will be used and decay
   frequently, so their evolution is tracked. The decay of recipe-based 
   IsoVectors is tracked through a parent-daughter relationship. Each parent
   and daughter is stored in a static container, and all IsoVectors in the 
   simulation using one of these recipes points to this static container. A 
   copy is only made if the composition deviates from the parent-daughter 
   chain through a + or - operation.

   Input file recipes can be either mass or atom (mole) based. 
   Recipes can be accessed a static map. For example:  
   @code 
    
   std::string recipe_name = "my-recipe-name"; 
   IsoVector* my_recipe = IsoVector::recipe(recipe_name); 
   
   @endcode 
 */
class IsoVector {
public:
  /* --- Constructors and Destructors --- */
  /**
     default constructor 
   */
  IsoVector(); 

  /**
     constructor given some previous composition 
     basis assumed to be mass-wise
     @param comp the previous composition 
   */
  IsoVector(composition* comp); 

  /**
     constructor given some initial composition 
     basis assumed to be mass-wise
     @param initial_comp the initial composition 
   */
  IsoVector(CompMap* initial_comp); 

  /**
     constructor given some initial composition and some
     basis
     @param initial_comp the initial composition 
     @param atom true if atom-basis, false if mass basis
   */
  IsoVector(CompMap* initial_comp, bool atom); 

  /**
     default destructor
   */
  ~IsoVector();
  /* --- */

  /* --- Operators  --- */
  /**
     Adds like isotopes 
   */
  IsoVector operator+ (IsoVector rhs_vector);

  /**
     Subtracts like isotopes 
      
     @exception CycRangeException thrown if subtraction results in a 
     negative quantity for any isotope. 
   */
  IsoVector operator- (IsoVector rhs_vector);

  /**
     Compares quantities of like isotopes returning true only if 
     quantity for every isotope is equal. 
   */
  bool operator== (IsoVector rhs_vector);

  /**
     multiplication operators
   */
  friend IsoVector operator* (const IsoVector &v, double factor);
  friend IsoVector operator* (double factor, const IsoVector &v);
  friend IsoVector operator* (const IsoVector &v, int factor);
  friend IsoVector operator* (int factor, const IsoVector &v);

  /**
     division operators
   */
  friend IsoVector operator/ (const IsoVector &v, double factor);
  friend IsoVector operator/ (const IsoVector &v, int factor);
  /* --- */

  /* --- Instance Interaction  --- */ 
  /**
     the current state id 
   */
  int stateID();

  /**
     Return the mass-based composition 
   */
  CompMap* mass_comp();

  /**
     returns the mass normalizer for the IsoVector's composition_
   */
  double mass_normalizer();

  /**
     returns the decay time for the IsoVector's composition_
   */
  int decay_time();

  /**
     returns the parent of the IsoVector's composition_
   */
  composition* parent();

  /**
     calls minimizeComposition() on composition_
   */    
  void minimizeComposition();

  /**
     multiply the mass_normalizer by a factor; used in multiplication
   */
  void multMassNormBy(double factor);

  /**
     Return the mass fraction of an isotope in the composition
   */
  double massFraction(Iso tope);

  /**
     Return the mass fraction of an isotope in a composition
   */
  static double massFraction(Iso tope, composition* c);

  /**
     returns the atom fraction of an isotope in the composition
   */
  double atomFraction(Iso tope);

  /**
     Return the mass fraction of an isotope in a composition
   */
  static double atomFraction(Iso tope, composition* c);

  /**
     Returns true if the given isotope's number density is less than the 
     conservation of mass tolerance. 
      
     @param tope the isotope in question 
     @return true iff nd(tope) == 0 
   */
  bool isZero(Iso tope);

  /**
     Decays this Material object for the given change in time and 
     updates its composition map with the new number densities. 
      
     @param time_change the number of months to decay 
   */
  void executeDecay(double time_change);
  /* --- */

  /* --- Global Interaction  --- */
  /**
     loads the recipes from the input file 
   */
  static void load_recipes();

  /**
     loads a specific recipe 
   */
  static void load_recipe(xmlNodePtr cur);
  
  /**
     logs a new recipe with the simulation
     - logs recipe with BookKeeper
   */
  static void logRecipe(composition* recipe);

  /**
     logs a new named recipe with the simulation
     - adds recipe to IsoVector's static containers
     - calls the logRecipe() method
   */
  static void logRecipe(std::string name, composition* recipe);

  /**
     logs a new named recipe with the simulation
     - sets the parent of and decay time child
     - calls the logRecipe() method
     @param t_i -> initial time of decay
     @param t_i -> final time of decay
   */
  static void logRecipeDecay(composition* parent, composition* child, 
                             int t_i, int t_f);
  /**
     adds recipe to containers tracking decayed recipes
   */
  static void storeDecayableRecipe(composition* recipe);
  
  /**
     checks if the recipe has been logged (i.e. it exists in the simulation)
   */
  static bool recipeLogged(std::string name);

  /**
     the total number of recipes 
   */
  static int recipeCount(); 

  /**
     accessing a recipe 
   */
  static composition* recipe(std::string name);

  /**
     accessing a set of decay times 
   */
  static decay_times* decayTimes(composition* parent);

  /**
     accessing the daughters of a parent
   */
  static DaughterMap* Daughters(composition* parent);

  /**
     accessing a specific daughter 
   */
  static composition* Daughter(composition* parent, int time);

  /**
     add a daughter to a map of daughters
   */
  static void addDaughter(DaughterMap* daughters, composition* child, int time);

  /**
     whether or not this composition is logged as a recipe
  */
  bool logged();
  /* --- */

  /* --- Isotope Wikipedia  --- */ 
  /**
     Returns the atomic number of the isotope with the given identifier. 
      
     @param tope the isotope whose atomic number is being returned 
     @return the atomic number 
   */
  static int getAtomicNum(Iso tope);

  /**
     Returns the mass number of the isotope with the given identifier. 
      
     @param tope the isotope whose mass number is being returned 
     @return the mass number 
   */
  static int getMassNum(Iso tope);
  /* --- */

  /* --- Printing Functionality  --- */
  /**
     print all recipes 
   */
  static void printRecipes();

  /**
     print the details of a composition 
   */
  static void print(composition* c);

  /**
     print the details of this IsoVector 
   */
  void print();
  /* --- */

 private:
  /* --- Initializations  --- */ 
  /**
     initialize any required members
   */
  void init();

  /**
     Turns a list of atom-based compositions
     to mass-based
   */
  void massify(CompMap* comp);

  /**
     Turns a list of mass-based compositions
     to atom-based
   */
  void atomify(CompMap* comp);

  /**
     set's the composition for this isovector
   */    
  void setComposition(composition* c);

  /**
     constructs a composition out of comp
     and calls setComposition()
   */    
  void setComposition(CompMap* comp);

  /**
     sets the composition for a mass or atom based
     composition. if atom == true, then massify()
     is called. setComposition() for the mass-based
     case is then called.
   */    
  void setComposition(CompMap* comp, bool atom);

  /**
     determines the mass/atom normalizers for a composition
   */    
  std::pair<double,double> getNormalizers(CompMap* comp);

  /**
     adjusts the composition such that the mass normalizer
     is equal to unity
   */    
  void minimizeComposition(composition* c);

  /**
     multiply the mass_normalizer of c by a factor; 
     used in multiplication
   */
  void multMassNormBy(composition* c,double factor);
  /* --- */

  /* --- Instance Interaction  --- */ 
  /**
     Stores the next available state ID 
   */
  static int nextStateID_;

  /**
     a pointer to the isovector's composition
   */
  composition* composition_;

  /**
     deletes the composition_ pointer and composition_'s
     CompMap of mass fractions, if it is not logged
   */
  void delete_comp();

  /**
     Decays a composition, assumed to be of mass-basis, for a given time. 
      
     @param time_change the number of months to decay 
     @param mass_comp composition to decay
   */
  void executeDecay(double time_change, composition* mass_comp);
  /* --- */

  /* --- Global Interaction  --- */
  /**
     a memory space for new iso vectors to point to
   */
  static CompMap* init_comp_;

  /**
     a container of recipes 
   */
  static RecipeMap recipes_;

  /**
     a container of recipes in each decay chain
   */
  static DecayChainMap decay_chains_;

  /**
     a container of decay times that recipes have gone through
   */
  static DecayTimesMap decay_times_;
  /* --- */

  /* --- Isotope Wikipedia  --- */
  /**
     calls validateIsotopeNumber() and validateFraction() 
     for each isotope and fraction value in this IsoVector's 
     composition
   */
  void validateComposition();

  /**
     throws an error if fraction < 0.0
   */
  static void validateFraction(double fraction);

  /**
     Used to determine validity of isotope defnition. 
     @param tope isotope identifier 
     @exception thrown if isotope identifier is invalid 
   */
  static void validateIsotopeNumber(Iso tope);
  /* --- */
 
  /* --- Printing Functionality  --- */
  /**
     return a pointer to a vector of the composition as strings 
     @return the composition string vector 
   */
  static std::vector<std::string>* compStrings(composition* c);

  /**
     used by print() to 'hide' print code when logging is not desired 
   */
  static std::string detail(composition* c);
  /* --- */
 

 /* -- Output Database Interaction  -- */ 
 public:
  /**
     the isotopics output database Table 
   */
  static table_ptr iso_table;
  
  /* /\** */
  /*    return the agent table's primary key  */
  /*  *\/ */
  /* primary_key_ref pkref(); */
  
  /**
     returns true if a new state was recorded, false if already in db
  */
  void recordState();
  
 private:
  /**
     Define the database table on the first Message's init 
   */
  static void define_table();

  /**
     Add an isotopic state to the table 
   */
  static void addToTable(composition* comp);

  /* /\** */
  /*    Store information about the transactions's primary key  */
  /*  *\/ */
  /* primary_key_ref pkref_; */
 /* -- */ 
};

#endif
