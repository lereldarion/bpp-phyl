//
// File: ApplicationTools.h
// Created by: jdutheil <Julien.Dutheil@univ-montp2.fr>
// Created on: Sun Dec 14 09:36:26 2003
//

#ifndef _APPLICATIONTOOLS_H_
#define _APPLICATIONTOOLS_H_

#include "Tree.h"
#include "SubstitutionModel.h"
#include "HomogeneousTreeLikelihood.h"

// From Utils:
#include <Utils/TextTools.h>
#include <Utils/StringTokenizer.h>

// From NumCalc:
#include <NumCalc/DiscreteDistribution.h>

// From SeqLib:
#include <Seq/SiteContainer.h>
#include <Seq/VectorSiteContainer.h>

// From the STL:
#include <string>
#include <map>
#include <iostream>
using namespace std;

/**
 * @brief This class provides some common tools for applications.
 *
 * The functions parse some option file, create corresponding objects and send
 * a pointer toward it.
 * 
 * The option files are supposed to follow this simple format:<br />
 * <code>parameterName = parameterContent</code><br />
 * with one parameter per line.
 *
 * In files, shell comments: <code> # my comment line here </code>,
 * C comments: <code> / * my comment block here * / </code>
 * and C++ comments: <code> // my comment line here </code> are allowed, and ignored while parsing.
 */
class ApplicationTools
{
	public:
		/**
		 * @brief The output stream where errors have to be displayed.
		 */
		static ostream & error;
		/**
		 * @brief The output stream where messages have to be displayed.
		 */
		static ostream & message;
		/**
		 * @brief The output stream where warnings have to be displayed.
		 */
		static ostream & warning;

	
	public:
		ApplicationTools();
		virtual ~ApplicationTools();
	
	public:

		/**
		 * @brief Tell if a parameter have been specified.
		 *
		 * @param parameterName The name of the parameter.
		 * @param params        The parameter list.
		 * @return True is the parameter of specified name is in the list.
		 */
		static bool parameterExists(const string & parameterName, map<string, string> & params);
	
		/**
		 * @brief Get a double parameter.
		 *
		 * @param parameterName    The name of the corresponding parameter.
		 * @param params           The attribute map where options may be found.
		 * @param defaultValue     The default value to use if the parameter is not found.
		 * @param suffix           A suffix to be applied to the parameter name.
		 * @param suffixIsOptional Tell if the suffix is absolutely required.
		 * @param warn             Tell if a warning must be sent in case the parameter is not found.
		 * @return The corresponding value.
		 */
		static double getDoubleParameter(
			const string & parameterName,
			map<string, string> & params,
			double defaultValue,
			const string & suffix = "",
			bool suffixIsOptional = true,
			bool warn = true);
	
		/**
		 * @brief Get an integer parameter.
		 *
		 * @param parameterName    The name of the corresponding parameter.
		 * @param params           The attribute map where options may be found.
		 * @param defaultValue     The default value to use if the parameter is not found.
		 * @param suffix           A suffix to be applied to the parameter name.
		 * @param suffixIsOptional Tell if the suffix is absolutely required.
		 * @param warn             Tell if a warning must be sent in case the parameter is not found.
		 * @return The corresponding value.
		 */
		static int getIntParameter(
			const string & parameterName,
			map<string, string> & params,
			int defaultValue,
			const string & suffix = "",
			bool suffixIsOptional = true,
			bool warn = true);
	
		/**
		 * @brief Get a string parameter.
		 *
		 * @param parameterName    The name of the corresponding parameter.
		 * @param params           The attribute map where options may be found.
		 * @param defaultValue     The default value to use if the parameter is not found.
		 * @param suffix           A suffix to be applied to the parameter name.
		 * @param suffixIsOptional Tell if the suffix is absolutely required.
		 * @param warn             Tell if a warning must be sent in case the parameter is not found.
		 * @return The corresponding value.
		 */
		static string getStringParameter(
			const string & parameterName,
			map<string, string> & params,
			string defaultValue,
			const string & suffix = "",
			bool suffixIsOptional = true,
			bool warn = true);

		/**
		 * @brief Get a boolean parameter.
		 *
		 * @param parameterName    The name of the corresponding parameter.
		 * @param params           The attribute map where options may be found.
		 * @param defaultValue     The default value to use if the parameter is not found.
		 * @param suffix           A suffix to be applied to the parameter name.
		 * @param suffixIsOptional Tell if the suffix is absolutely required.
		 * @param warn             Tell if a warning must be sent in case the parameter is not found.
		 * @return The corresponding value.
		 */
		static bool getBooleanParameter(
			const string & parameterName,
			map<string, string> & params,
			bool defaultValue,
			const string & suffix = "",
			bool suffixIsOptional = true,
			bool warn = true);

		/**
		 * @brief Get a file path.
		 *
		 * @param parameter        The name of the corresponding parameter.
		 * @param params           The attribute map where options may be found.
		 * @param isRequired       Tell if this path is strictly required or is optional
		 * (in the first case, if the parameter is not found, the programm will
		 * send an error and exit).
		 * @param mustExist  Tell if the corresponding file must already exist.
		 * @param suffix           A suffix to be applied to the parameter name.
		 * @param suffixIsOptional Tell if the suffix is absolutely required.
		 */
		static string getAFilePath(
			const string & parameter,
			map<string, string> & params,
			bool isRequired = true,
			bool mustExist = true,
			const string & suffix = "",
			bool suffixIsOptional = false);

		/**
		 * @brief Get a vector.
		 *
		 * @param parameterName    The name of the corresponding parameter.
		 * @param params           The attribute map where options may be found.
		 * @param defaultValue     The default value to use if the parameter is not found.
		 * @param suffix           A suffix to be applied to the parameter name.
		 * @param suffixIsOptional Tell if the suffix is absolutely required.
		 * @param warn             Tell if a warning must be sent in case the parameter is not found.
		 * @return The corresponding value.
         */
		 template<class T> static vector<T> getVectorParameter(
			const string & parameterName,
			map<string, string> & params,
            char separator,
			const string & defaultValue,
			const string & suffix = "",
			bool suffixIsOptional = true,
			bool warn = true)
		{
            string s = getStringParameter(parameterName, params, defaultValue, suffix, suffixIsOptional, warn);
            StringTokenizer st(s, TextTools::toString(separator));
            unsigned int n = st.numberOfRemainingTokens();
            vector<T> v(n);
            for(unsigned int i = 0; i < n; i++) {
                v[i] = TextTools::fromString<T>(st.nextToken());
            }
            return v;
        }

		
		/**
		 * @name Output methods.
		 *
		 * @{
		 */
		
		/**
		 * @brief Print a message.
		 *
		 * @param text The text of the message.
		 */
		static void displayMessage(const string & text);
		
		/**
		 * @brief Print a error message.
		 *
		 * @param text The text of the message.
		 */
		static void displayError(const string & text);
		
		/**
		 * @brief Print a warning message.
		 *
		 * @param text The text of the message.
		 */
		static void displayWarning(const string & text);
		
		/**
		 * @brief Print a task message.
		 *
		 * Display the message and flush the buffer, but do not end current line.
		 
		 * @param text The text of the message.
		 */
		static void displayTask(const string & text);
		
		/**
		 * @brief Print a task ended message.
		 *
		 * Print "Done." and go to next line.
		 *
		 * @param text The text of the message.
		 */
		static void displayTaskDone();
		
		/**
		 * @brief Print a result message.
		 *
		 * Result will be alined to 30 character from the begining of the message.
		 * ex: text = "Here is what you get:" and result = "THAT" gives
		 * "Here is what you get:          THAT".
		 *		
		 * @param text   The text of the message.
		 * @param result The result.
		 */
		static void displayResult(const string & text, const string & result);

		/** @} */

		/**
		 * @brief Build an Alphabet object according to options.
		 *
		 * Options used are:
		 * - alphabet = [DNA|RNA|Protein], the alphabet type to use.
		 *
		 * @param params  The attribute map where options may be found.
		 * @param suffix  A suffix to be applied to each attribute name.
		 * @param suffixIsOptional Tell if the suffix is absolutely required.
		 * @param verbose Print some info to the 'message' output stream.
		 * @return A new Alphabet object according to options specified.
		 */
		static Alphabet * getAlphabet(
			map<string, string> & params,
			const string & suffix = "",
			bool suffixIsOptional = true,
			bool verbose = true);
	
		/**
		 * @brief Build a Tree object according to options.
		 *
		 * Only newick format is supported for now.
		 * Options used are:
		 * - tree.file = file_path, the path of the file to parse.
		 *
		 * @param params  The attribute map where options may be found.
		 * @param suffix  A suffix to be applied to each attribute name.
		 * @param suffixIsOptional Tell if the suffix is absolutely required.
		 * @param verbose Print some info to the 'message' output stream.
		 * @return A new Tree object according to options specified.
		 */
		static Tree * getTree(
			map<string, string> & params,
			const string & suffix = "",
			bool suffixIsOptional = true,
			bool verbose = true);
	
		/**
		 * @brief Build a SequenceContainer object according to options.
		 *
		 * Options used are:
		 * - sequence.format = [Fasta|Mase|Phylip], the format of the sequence file.
		 * - sequence.file = file_path, the path of the file to parse.
		 * Options depending on other options:
		 * - If Phylip format is to be used:
		 *   - sequence.format_phylip.order = [interleaved|sequential].
		 *   - sequence.format_phylip.ext   = [classic|extended].
		 * - If Mase format is to be used:
		 *   - sequence.format_mase.site_selection = name of the selection.
		 *
		 * @param params  The attribute map where options may be found.
		 * @param suffix  A suffix to be applied to each attribute name.
		 * @param suffixIsOptional Tell if the suffix is absolutely required.
		 * @param verbose Print some info to the 'message' output stream.
		 * @return A new VectorSiteContainer object according to options specified.
		 */
		static VectorSiteContainer * getSiteContainer(
			const Alphabet * alpha,
			map<string, string> & params,
			const string & suffix = "",
			bool suffixIsOptional = true,
			bool verbose = true);
			
		/**
		 * @brief retrieves sites suitable for the analysis.
		 *
		 * Options used are:
		 * - sequence.sites_to_use = [complete|nogap].
		 *
		 * If the 'complete' option is used, only fully resolve site will be taken
		 * into account.
		 * If the 'nogap' option is used, only sites without gap will be taken into
		 * account.
		 *
		 * @param params  The attribute map where options may be found.
		 * @param suffix  A suffix to be applied to each attribute name.
		 * @param suffixIsOptional Tell if the suffix is absolutely required.
		 * @param verbose Print some info to the 'message' output stream.
		 * @return A new VectorSiteContainer object containing sites of interest.
		 */
		static VectorSiteContainer * getSitesToAnalyse(
			const SiteContainer & allSites,
			map<string, string> & params,
			string suffix = "",
			bool suffixIsOptional = true,
			bool verbose = true);
	
		/**
		 * @brief Build a SubstitutionModel object according to options.
		 *
		 * Options used are:
		 * - model = [JCnuc|K80|T92|HKY85|TN93], the substitution model to use.
		 * Options depending on the model specified:
		 * - If K80, T92 or HKY85 is to be used:
		 *   - kappa The transition/transversion ratio.
		 * - If T92 format is to be used:
		 *   - theta The GC ratio, or
		 * - If HKY or TN93 is to be used:
		 *   - piA, piT, piC and piG: equilibrum frequencies.
		 * - If TN93 is to be used:
		 *   - kappa1, kappa2 The transition/transversion ratios.
		 * - If TN93, HKY85 or T92 is to be used:
		 *   - model.use_observed_freq Tell if we must use the observed frequences. 
		 *
		 * @param data    The SiteContainer for which the substitution model is designed.
		 * @param params  The attribute map where options may be found.
		 * @param suffix  A suffix to be applied to each attribute name.
		 * @param suffixIsOptional Tell if the suffix is absolutely required.
		 * @param verbose Print some info to the 'message' output stream.
		 * @return A new SubstitutionModel object according to options specified.
		 */
		static SubstitutionModel * getSubstitutionModel(
			const SiteContainer & data, 
			map<string, string> & params,
			const string & suffix = "",
			bool suffixIsOptional = true,
			bool verbose = true);
	
		/**
		 * @brief Build a DiscreteDistribution object according to options.
		 *
		 * Options used are:
		 * - rate_distribution = [constant|gamma], the distribution to use.
		 * Options depending on other options:
		 * - If 'gamma' is selected:
		 *   - rate_distribution_gamma.alpha = the shape of the distribution
		 *   - rate_distribution.classes_number = the number of categories to be used.
		 *
		 * @param params  The attribute map where options may be found.
		 * @param suffix  A suffix to be applied to each attribute name.
		 * @param suffixIsOptional Tell if the suffix is absolutely required.
		 * @param verbose Print some info to the 'message' output stream.
		 * @return A new DiscreteDistribution object according to options specified.
		 */
		static DiscreteDistribution * getRateDistribution(
			map<string, string> & params,
			string suffix = "",
			bool suffixIsOptional = true,
			bool verbose = true);
	
		/**
		 * @brief Optimize parameters according to options.
		 *
		 * Options used are:
		 * - optimization.method = [simplex|powell|simplex+powell|simplex+brent|powell+brent], the kind of optimization to perform.
		 * - optimization.message_handler = [std, file_path]
		 *   A path to a specific path (existing will be overwritten) or std for use
		 *   of the standard output.
		 * - optimization.profiler = [std, file_path], idem for the profiling (history
		 *   of all functions evaluations).
		 * - optimization.max_number_f_eval = The maximum number of function evaluation.
		 * - optimization.tolerance = The tolerance parameter (when to stop the optimization).
		 * - optimization.scale_first = Tell if we must scale the tree first.
		 * - optimization.ignore_parameter = A coma-separated list of parameter
		 *   names to ignore in the optimizing process.
		 * Options depending on other options:
		 * - 'simplex+powell' method additional options:
		 *   - sp_tol = Tolerance for the downhill simplex method. 
		 *     When this tolerance is reached, then switch to Powell' method,
		 *     if max_number_f_eval has not been reached before.
		 *     The general tolerance parameter is used for powell's method.
		 * - If optimization.scale_first is set to true:
		 *   - scale_opt.tolerance = The tolerance of the scaling alogrithm.
		 *   - scale_opt.max_number_f_eval = the maximum number of function evaluations
		 *     for the scaling algorithm.
		 * - If 'simplex+brent' and 'powell+brent' additional option:
		 *   - alpha_profiler = The profiler for the alpha parameter which is estimated separately.
		 *
		 * @param tl      The TreeLikelihood function to optimize.
		 * @param params  The attribute map where options may be found.
		 * @param suffix  A suffix to be applied to each attribute name.
		 * @param suffixIsOptional Tell if the suffix is absolutely required.
		 * @param verbose Print some info to the 'message' output stream.
		 * @throw Exception Any exception that may happen during the optimization process.
		 */
		static void optimizeParameters(
			TreeLikelihood * tl,
			map<string, string> & params,
			const string & suffix = "",
			bool suffixIsOptional = true,
			bool verbose = true)
			throw (Exception);
		
		/**
		 * @brief This function prints the options available for substitution models.
		 */
		static void printSubstitutionModelHelp();
		
		/**
		 * @brief This function prints the options available for rate distributions.
		 */
		static void printRateDistributionHelp();
		
		/**
		 * @brief This function prints the options available for optimization.
		 */
		static void printOptimizationHelp();
		
		/**
		 * @brief Write a tree according to options.
		 *
		 * Options used are:
		 * - output.tree = file_path, the file where to put the tree.
		 *
		 * NB: only Newick format is supported for now. 
		 *
		 * @param tree    The tree to write.
		 * @param params  The attribute map where options may be found.
		 * @param suffix  A suffix to be applied to each attribute name.
		 * @param verbose Print some info to the 'message' output stream.
		 */
		static void writeTree(
			const Tree & tree,
			map<string, string> & params,
			const string & suffix = "",
			bool verbose = true);
		
		/**
		 * @brief Write a sequence file according to options.
		 *
		 * Optioins used are:
		 * - output.sequence.format = [Fasta|Mase|Phylip], the format of the sequence file.
		 * - output.sequence.file = file_path, the path of the file to parse.
		 * - output.sequence.length = the max number of chars on a line.
		 * Options depending on other options:
		 * - If Phylip format is to be used:
		 *   - output.sequence.format_phylip.order = [interleaved|sequential].
		 *   - output.sequence.format_phylip.ext   = [classic|extended].
		 *
		 * @param sequences The sequences to write.
		 * @param params  The attribute map where options may be found.
		 * @param suffix  A suffix to be applied to each attribute name.
		 * @param verbose Print some info to the 'message' output stream.
		 */
		static void writeSequenceFile(
			const SequenceContainer & sequences,
			map<string, string> & params,
			const string & suffix = "",
			bool verbose = true);

};


#endif	//_APPLICATIONTOOLS_H_