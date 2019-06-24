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

        static void Main(string[] args)
        {
            Console.WriteLine("Dumping Provides! Ignored Providers: " + string.Join(", ", args));

            if (!Directory.Exists(manifestsDirectory))
            {
                Directory.CreateDirectory(manifestsDirectory);
            }

            List<string> providers = null;

            try
            {
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
                }
            }
        }
    }
}
