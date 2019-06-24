using Microsoft.Diagnostics.Tracing.Parsers;
using Microsoft.Diagnostics.Tracing.Session;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.Eventing.Reader;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DotNetEtwProviderDocs
{
    class Program
    {
        static string manifestsDirectory = "Manifests";

        /// <summary>
        /// Dump manifest based providers in the current machine
        /// </summary>
        /// <param name="args">Providers to ignore (some providers cause the dump process to crash so add them in the command line)</param>
        static void Main(string[] args)
        {
            Console.WriteLine("Dumping Provides! Ignored Providers: [" + string.Join(", ", args) + "]");

            if (!Directory.Exists(manifestsDirectory))
            {
                Directory.CreateDirectory(manifestsDirectory);
            }

            List<string> providers = null;

            try
            {
                // Get all installed providers
                providers = EventLogSession.GlobalSession.GetProviderNames().ToList();
            }
            catch (Exception e)
            {
                Console.WriteLine("Could Not Get ETW Provider List: " + e.ToString());
                Environment.Exit(0);
            }

            foreach (string providerName in providers)
            {
                if (IsIgnoredProvider(args, providerName))
                {
                    Console.WriteLine("[+++] Ignoring " + providerName);
                    continue;
                }

                try
                {
                    DumpProvider(providerName);
                }
                catch (Exception)
                {
                    // Currently ignore errors
                    // Most of the errors are caused because of MOF class providers which does not have a manifest
                    Console.WriteLine("[!!] Could not dump " + providerName);
                    
                }
            }
        }

        /// <summary>
        /// Dump the provider if the manifest does not exist 
        /// If the provider is not manifest based exception will be raised
        /// Uses the same function perfview uses to dump the providers
        /// </summary>
        /// <param name="providerName">name of the ETW providers</param>
        static void DumpProvider(string providerName)
        {
            string manifestFileName = providerName + ".xml";


            string manifestFilePath = Path.Combine(manifestsDirectory, manifestFileName);

            if (File.Exists(manifestFilePath))
            {
                return;
            }

            Console.WriteLine("[+] Writing " + manifestFileName);
            
            string manifest = RegisteredTraceEventParser.GetManifestForRegisteredProvider(providerName);

            File.WriteAllText(manifestFilePath, manifest);
        }

        static bool IsIgnoredProvider(string[] args, string providerName)
        {
            foreach (string arg in args)
            {
                if (providerName.Contains(arg))
                {
                    return true;
                }
            }

            return false;
        }
    }
}
