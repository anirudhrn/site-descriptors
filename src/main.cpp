#include <iostream>
#include <casm/CASM_global_definitions.hh>
#include <casm/clex/PrimClex.hh>
#include <casm/app/ProjectSettings.hh>
#include <casm/casm_io/jsonParser.hh>
#include <casm/clex/ConfigDoF.hh>
#include <fstream>
#include <casm/container/Array.hh>
#include <casm/clex/Correlation.hh>

using namespace CASM;
using namespace std;
/// \brief Returns unrolled per-site correlations using 'clexulator'. Supercell needs a correctly populated neighbor list.
jsonParser correlations_mat_local(const ConfigDoF &configdof, const Supercell &scel, Clexulator &clexulator) {

  int scel_vol = scel.volume();
  int basis_size = scel.basis_size();
  Eigen::MatrixXd correlations_mat = Eigen::MatrixXd::Zero(clexulator.corr_size(), scel_vol*basis_size);
  clexulator.set_config_occ(configdof.occupation().begin());
  vector<int> b_idx(scel.num_sites(),-5);
  //Holds contribution to global correlations from a particular neighborhood
  Eigen::VectorXd tcorr = Eigen::VectorXd::Zero(clexulator.corr_size());
  
  for(int v = 0; v < scel.num_sites(); v++) {
    b_idx[v] = scel.get_b(v);
    //Point the Clexulator to the right neighborhood
    clexulator.set_nlist(scel.nlist().sites(scel.nlist().unitcell_index(v)).data());
    //Fill up contributions
    clexulator.calc_point_corr(scel.get_b(v),tcorr.data());
    correlations_mat.col(v) = tcorr;
  }
  jsonParser config_correlations;
  config_correlations["corr_matrix"] = correlations_mat;
  config_correlations["b"] = b_idx;
  return config_correlations;
}

int main(int argc, char *argv[])
{
    print_splash(cout);

    if(argc!=3){
        cerr<<"Usage:"<<argv[0]<<" <CASM project location> <output filename>"<<endl;
        return 1;
    }
    
    fs::path prim_path(argv[1]);
    fs::path output_correlation_filename(argv[2]);
    
    // Load in the primclex
    PrimClex pclex=PrimClex(fs::path(prim_path));
    if(!pclex.has_clexulator(pclex.settings().default_clex())){
        cout<<"You have not initialized a clexulator for this project. Please run "
                 <<"'casm bset -u' before you run this program again."<<endl;
        exit(666);
    }

    Clexulator clex = pclex.clexulator(pclex.settings().default_clex());
    jsonParser data;
  
    for(auto config_iter = pclex.selected_config_cbegin(); config_iter!=pclex.selected_config_cend(); ++config_iter ){
        auto con_name = config_iter->name();
        cout<<"Working on : "<<con_name<<endl;
        data[con_name] = correlations_mat_local((*config_iter).configdof(),(*config_iter).get_supercell(),clex);
    }
    data.write(output_correlation_filename);
    return 0;
}
