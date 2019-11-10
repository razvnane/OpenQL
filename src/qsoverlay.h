/**
 * @file   qsoverlay.h
 * @date   11/2019
 * @author Diogo Valada
 * @brief  Prepares the quantumsim circuits using the qsoverlay format
 */

#include <vector>
#include <string>
#include <kernel.h>
#include <gate.h>


//Only support for DiCarlo setup atm
void write_qsoverlay_program( std::string prog_name, size_t num_qubits,
        std::vector<ql::quantum_kernel>& kernels, const ql::quantum_platform & platform, std::string suffix, size_t ns_per_cycle, bool compiled)
    {
        IOUT("Writing scheduled QSoverlay program");
        ofstream fout;
        string qfname( ql::options::get("output_dir") + "/" + prog_name + "_quantumsim_" + suffix + ".py");
        DOUT("Writing scheduled QSoverlay program " << qfname);
        IOUT("Writing scheduled QSoverlay program " << qfname);
        fout.open( qfname, ios::binary);
        if ( fout.fail() )
        {
            EOUT("opening file " << qfname << std::endl
                     << "Make sure the output directory ("<< ql::options::get("output_dir") << ") exists");
            return;
        }

        fout << "# Quantumsim (via Qsoverlay) program generated by OpenQL\n"
             << "# Please modify at your will to obtain extra information from Quantumsim\n\n";

        fout << "import numpy as np\n"
			 << "from qsoverlay import DiCarlo_setup\n"
			 << "from qsoverlay.circuit_builder import Builder\n";
			
		fout << "import quantumsim.sparsedm as sparsedm\n"
             << "\n"
             << "# print('GPU is used:', sparsedm.using_gpu)\n"
             << "\n"
             << "\n";

				
				
		//Gate correspondence
		std::map <std::string, std::string> gate_map = {
			{"prepz", "prepz"},
			{"x", "X"},
			{"x45", "RX"},
			{"x90", "RX"},
			{"xm45", "RX"},
			{"xm90", "RX"},
			{"y", "Y"},
			{"y45", "RY"},
			{"y90", "RY"},
			{"ym45", "RY"},
			{"ym90", "RY"},
			{"h", "H"},
			{"cz", "CZ"},
			{"measure", "Measure"},
		};

		if (not compiled)
			gate_map["cnot"] = "CNOT";

		std::map <std::string, std::string> angles = {
			{"x45", "np.pi/4"},
			{"x90", "np.pi/2"},
			{"xm45", "-np.pi/4"},
			{"xm90", "-np.pi/2"},
			{"y45", "np.pi/4"},
			{"y90", "np.pi/2"},
			{"ym45", "-np.pi/4"},
			{"ym90", "-np.pi/2"},
		};

		//Create qubit list

		std::string qubit_list = "";
		for (size_t qubit=0; qubit < num_qubits; qubit++)
		{
			qubit_list += "'";
			qubit_list += (qubit != num_qubits-1) ? (std::to_string(qubit) + "', ") : (std::to_string(qubit) + "'");
		}

		//Circuit creation
		fout << "\n#Now the circuit is created\n"

			 << "\ndef circuit_generated(noise_flag, setup_name = 'DiCarlo_setup'):\n"
			 << "	qubit_list = [" + qubit_list + "]\n"
			 << "	if setup_name == 'DiCarlo_setup':\n"
			 << "		setup = DiCarlo_setup.quick_setup(qubit_list, noise_flag = noise_flag)\n"
			 << "	b = Builder(setup)\n";


		//Circuit creation: Add gates
		for (auto & gate: kernels.front().c)
		{
			std::string qs_name;
			try
			{
  				qs_name = gate_map.at(gate->name);
				// WOUT("Next gate: " + gate->name + " .... CORRECT");

			}
  			catch (exception& e)
  			{
				// WOUT("Next gate: " + gate->name + " .... WRONG");
				EOUT("Qsoverlay: unknown gate detected!: " + gate->name);
    			throw ql::exception("Qsoverlay: unknown gate detected!:"  + gate->name, false);
				
  			}

			IOUT(gate->name);
			if (gate->operands.size() == 1)
				IOUT("Gate operands: " + std::to_string(gate->operands[0]));
			else if (gate->operands.size() == 2)
				IOUT("Gate operands: " + std::to_string(gate->operands[0]) + ", " + std::to_string(gate->operands[1]));
			else
			{
				IOUT("GATE OPERANDS: Problem encountered");
			}
			

			fout << "	b.add_gate('" << qs_name  << "', " << "['" 
				 << std::to_string(gate->operands[0])
				 << (( gate->operands.size() == 1 ) ? "']" : ("', '" + std::to_string(gate->operands[1]) + "']"));
			
			
			//Add angles for the gates that require it
			if (qs_name == "RX" or qs_name == "RY")
				fout << ", angle = " << angles[gate->name];

			
			//Add gate timing, if circuit was compiled.
			if (qs_name == "prepz")
			{
				if (compiled)
					fout << ", time = " << std::to_string((gate->cycle-1)*ns_per_cycle + gate->duration);
			}

			else if (qs_name == "Measure")
			{
				fout << ", output_bit = " << "'" << gate->operands[0] << "_out'";
				if (compiled)
					fout << ", time = " << std::to_string((gate->cycle-1)*ns_per_cycle + gate->duration/4);
			}
			else
			{
				if (compiled)
					fout << ", time = " << std::to_string((gate->cycle-1)*ns_per_cycle + gate->duration/2);
			}
			fout << ")\n";
		}

		fout << "\n"
			 << "	b.finalize()\n"
			 << "	return b.circuit\n";

        fout.close();
        IOUT("Writing scheduled QSoverlay program [Done]");
    }
