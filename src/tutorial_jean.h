#ifndef TUTORIAL_JEAN_H_
#define TUTORIAL_JEAN_H_

#include "biodynamo.h"

namespace bdm {

  // 0. Define my custom cell MyCell, which extends Cell by adding extra data members: cell_color and can_divide
  BDM_SIM_OBJECT(MyCell, Cell) { // our object extends the Cell object
    BDM_SIM_OBJECT_HEADER(MyCellExt, 1, can_divide_, cell_colour_); // create the header with our new data member

    public:
      MyCellExt() {}
      MyCellExt(const std::array<double, 3>& position) : Base(position) {} // our creator

      // getter and setter for our new data member
      void SetCanDivide(bool d) { can_divide_[kIdx] = d; }
      bool GetCanDivide() { return can_divide_[kIdx]; }
      bool* GetCanDividePtr() { return can_divide_.data(); }

      void SetCellColour(int cellColour) { cell_colour_[kIdx] = cellColour; }
      int GetCellColour() { return cell_colour_[kIdx]; }
      int* GetCellColourPtr() { return cell_colour_.data(); }

    private:
      // declare new data member and define their type
      // private data can only be accessed by public function and not directly
      vec<bool> can_divide_;
      vec<int> cell_colour_;
  };

// 1. Define growth behaviour
  template <typename TSimulation = Simulation<>>
  struct GrowthModule : public BaseBiologyModule {
  GrowthModule() : BaseBiologyModule(gAllBmEvents) {}

    template <typename T>
      void Run(T* cell) {
        auto* sim = TSimulation::GetActive();
        auto* random = sim->GetRandom();

      if (cell->GetDiameter() < 8) {
        cell->ChangeVolume(400);

        std::array<double, 3> cell_movements{random->Uniform(-2, 2), random->Uniform(-2, 2), random->Uniform(-2, 2)}; // create an array of 3 ramdom numbers between -2 and 2
        cell->UpdatePosition(cell_movements); // update the cell mass location, ie move the cell
        cell->SetPosition(cell->GetPosition()); // set the cell position
      }

      else { // if diameter > 8

        if (random->Uniform(0, 1) <0.8 && cell->GetCanDivide()){
          auto daughter = cell->Divide();
          daughter->SetCellColour(cell->GetCellColour()); // daughter take the cell_colour_ value of her mother
          daughter->SetCanDivide(true); // the daughter will be able to divide
        }
        else {
          cell->SetCanDivide(false); // this cell won't divide anymore
        }
      }
    }

    ClassDefNV(GrowthModule, 1);
  };

// Define compile time parameter
  template <typename Backend>
    struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using BiologyModules = Variant<GrowthModule<>>; // add GrowthModule
  using AtomicTypes = VariadicTypedef<MyCell>; // use MyCell object
  };

template <typename TSimulation = Simulation<>>
inline int Simulate(int argc, const char** argv) {
  Simulation<> simulation(argc, argv);
  auto* rm = simulation.GetResourceManager();
  auto* random = simulation.GetRandom();
  auto* scheduler = simulation.GetScheduler();
  auto* param = simulation.GetParam();

  size_t nb_of_cells = 2400; // number of cells in the simulation
  double x_coord, y_coord, z_coord;

  param->bound_space_ = true;
  param->min_bound_ = 0;
  param->max_bound_ = 100; // cube of 100*100*100
  param->run_mechanical_interactions_ = true;

  auto cells = rm->template Get<MyCell>(); // create a structure to contain cells
  cells->reserve(nb_of_cells); // allocate the correct number of cell in our cells structure before cell creation

  for (size_t i = 0; i < nb_of_cells; ++i) {
    // our modelling will be a cell cube of 100*100*100
    x_coord = random->Uniform(param->min_bound_, param->max_bound_); // random double between 0 and 100
    y_coord = random->Uniform(param->min_bound_, param->max_bound_);
    z_coord = random->Uniform(param->min_bound_, param->max_bound_);

    MyCell cell({x_coord, y_coord, z_coord}); // creating the cell at position x, y, z
    // set cell parameters
    cell.SetDiameter(7.5);
    cell.SetCellColour((int) (y_coord/param->max_bound_*6)); // will vary from 0 to 5. so 6 different layers depending on y_coord

    cells->push_back(cell); // put the created cell in our cells structure
  }

  // create a cancerous cell, containing the BiologyModule GrowthModule
  MyCell cell({20, 50, 50});
  cell.SetDiameter(6);
  cell.SetCellColour(8);
  cell.SetCanDivide(true);
  cell.AddBiologyModule(GrowthModule<>());
  cells->push_back(cell); // put the created cell in our cells structure

  cells->Commit(); // commit cells

//  param->live_visualization_ = true; // allows live visualization
//  param->export_visualization_ = true; // allows export of visualization files
//  param->visualization_export_interval_ = 10; // export visualization files every 10 steps
//  param->visualize_sim_objects_["MyCell"] = std::set<std::string>{"diameter_", "cell_colour_"}; // add the data member diameter_ to the visualization objects


  // Run simulation
  scheduler->Simulate(400);

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

} // namespace bdm

#endif // TUTORIAL_JEAN_H_
