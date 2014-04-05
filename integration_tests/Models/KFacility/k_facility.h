#ifndef CYCLUS_INTEGRATION_TESTS_MODELS_K_FACILITY_H_
#define CYCLUS_INTEGRATION_TESTS_MODELS_K_FACILITY_H_

#include <string>
#include <set>
#include <vector>

#include "cyclus.h"

namespace cyclus {

class Context;

/**
  @class KFacility
  This facility is a source and consumer of some commodity with capacity.

  This FacilityModel is intended
  to be used for Cyclus trivial cycle integration tests.

  @section intro Introduction
  Place an introduction to the model here.

  @section modelparams Model Parameters
  Place a description of the required input parameters which define the
  model implementation.

  @section optionalparams Optional Parameters
  Place a description of the optional input parameters to define the
  model implementation.

  @section detailed Detailed Behavior
  Place a description of the detailed behavior of the model. Consider
  describing the behavior at the tick and tock as well as the behavior
  upon sending and receiving materials and messages.
  */
class KFacility
    : public cyclus::FacilityModel,
      public cyclus::CommodityProducer {

  /* --------------------
   * all FACILITYMODEL classes have these members
   * --------------------
   */
 public:
  /**
    Constructor for KFacility Class
    @param ctx the cyclus context for access to simulation-wide parameters
    */
  explicit KFacility(cyclus::Context* ctx);

  /**
    every model should be destructable
    */
  virtual ~KFacility();

  virtual std::string schema();

  virtual cyclus::Model* Clone();
  /**
    Initialize members related to derived module class

    @param qe a pointer to a QueryEngine object containing initialization data
    */
  virtual void InfileToDb(cyclus::QueryEngine* qe, cyclus::DbInit di);

  virtual void InitFrom(cyclus::QueryBackend* b);

  virtual void Snapshot(cyclus::DbInit di);

  virtual void InitInv(cyclus::Inventories& inv) {};

  virtual cyclus::Inventories SnapshotInv() {return cyclus::Inventories();}

  /**
    Initialize members for a cloned module.
    */
  virtual void InitFrom(KFacility* m);

  /**
    A verbose printer for the KFacility
    */
  virtual std::string str();

  /**
    The handleTick function specific to the KFacility.

    @param time the time of the tick
    */
  virtual void Tick(int time);

  /**
    The handleTick function specific to the KFacility.

    @param time the time of the tock
    */
  virtual void Tock(int time);

  /// @brief Responds to each request for this source facility's commodity.
  /// If a given request is more than this facility's capacity, it will offer
  /// its capacity.
  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
      GetMatlBids(const cyclus::CommodMap<cyclus::Material>::type&
                  commod_requests);

  /// @brief respond to each trade with a material made from this facility's
  /// recipe
  ///
  /// @param trades all trades in which this trader is the supplier
  /// @param responses a container to populate with responses to each trade
  virtual void GetMatlTrades(
      const std::vector< cyclus::Trade<cyclus::Material> >& trades,
      std::vector<std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >& responses);

  /// @brief Request Materials of their given commodity. Note
  /// that it is assumed the facility operates on a single resource type!
  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
      GetMatlRequests();

  /// @brief Request GenericResources of their given
  /// commodity. Note that it is assumed the facility operates on a single
  /// resource type!
  virtual std::set<cyclus::RequestPortfolio<cyclus::GenericResource>::Ptr>
      GetGenRsrcRequests();

  /// @brief Place accepted trade Materials in their Inventory
  virtual void AcceptMatlTrades(
      const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >& responses);

  /// @brief Place accepted trade Materials in their Inventory
  virtual void AcceptGenRsrcTrades(
      const std::vector< std::pair<cyclus::Trade<cyclus::GenericResource>,
      cyclus::GenericResource::Ptr> >& responses);
  /* --- */

  /* --- KFacility Members --- */
  /**
    @brief creates a material object to offer to a requester
    @param target the material target a request desires
    */
  cyclus::Material::Ptr GetOffer(const cyclus::Material::Ptr target) const;

  /**
    sets the output commodity name
    @param name the commodity name
    */
  inline void commodity(std::string name) {
    out_commod_ = name;
  }

  /// @return the output commodity
  inline std::string commodity() const {
    return out_commod_;
  }

  /**
    sets the capacity of a material generated at any given time step
    @param capacity the production capacity
    */
  inline void capacity(double capacity) {
    out_capacity_ = capacity;
    current_capacity_ = out_capacity_;
  }

  /// @return the production capacity at any given time step
  inline double capacity() const {
    return out_capacity_;
  }

  /**
    sets the name of the recipe to be produced
    @param name the recipe name
    */
  inline void recipe(std::string name) {
    recipe_name_ = name;
  }

  /// @return the name of the output recipe
  inline std::string recipe() const {
    return recipe_name_;
  }

  /// @return the current timestep's capacity
  inline double current_capacity() const {
    return current_capacity_;
  }

  /**
     add a commodity to the set of input commodities
     @param name the commodity name
   */
  inline void AddCommodity(std::string name) {
    in_commods_.push_back(name);
  }

  /**
    determines the amount to request
    */
  inline double RequestAmt() const {
    return in_capacity_;
  }

  /// @ return the conversion factor input
  inline double k_factor_in() const {
    return k_factor_in_;
  }

  /// sets the conversion factor input
  /// @param new conversion factor
  inline void k_factor_in(double k_factor){
    k_factor_in_ = k_factor;
  }

  /// @ return the conversion factor for output
  inline double k_factor_out() const {
    return k_factor_out_;
  }

  /// sets the conversion factor for output
  /// @param new conversion factor
  inline void k_factor_out(double k_factor){
    k_factor_out_ = k_factor;
  }

 private:
  /**
    This facility has one output commodity and one input commodity
    */
  std::string in_commod_;
  std::string out_commod_;

  /**
    Name of the recipe this facility uses.
    */
  std::string recipe_name_;

  /**
    The capacity is defined in terms of the number of units of the
    recipe that can be provided each time step.  A very large number
    can be provided to represent infinte capacity.
    In and out commodity capacities are defined
    */
  double in_capacity_;
  double out_capacity_;

  /**
    The capacity at the current time step
    */
  double current_capacity_;

  /**
    The price that the facility will charge for its output commodity.
    Units vary and are in dollars per inventory unit.
    */
  double commod_price_;

  /**
    Memebers for request operations.
    */
  std::vector<std::string> in_commods_;
  cyclus::ResourceBuff inventory_;

  /**
    Conversion factors
    */
  double k_factor_in_;
  double k_factor_out_;
  /* --- */
};

}  // namespace cyclus

#endif  // CYCLUS_INTEGRATION_TESTS_MODELS_K_FACILITY_H_