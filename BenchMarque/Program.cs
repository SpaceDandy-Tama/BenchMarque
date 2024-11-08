using System;
using System.Diagnostics;
using System.Threading.Tasks;

namespace TestSMT
{
    class Program
    {
        static void Main(string[] args)
        {
            long countPerLogicalProcessor = 1000000000;

            Console.WriteLine($"Logical Processors: {Environment.ProcessorCount}");
#if DEBUG
            Console.WriteLine($"{countPerLogicalProcessor} calculations per logical processor");
#endif

            long singleScore = SingleThreadedTest(countPerLogicalProcessor);

            if (Environment.ProcessorCount > 0)
            {
                long multiScore = MultiThreadedTest(countPerLogicalProcessor);
                Console.WriteLine($"Ratio: {Math.Round((double)multiScore/singleScore, 2, MidpointRounding.AwayFromZero)}x");
            }
            else
            {
                Console.WriteLine("Omited MultiThreaded Test because there is no point.");
            }
        }

        static long SingleThreadedTest(long count)
        {
            Stopwatch sw = new Stopwatch();

            //There is no reason to use actual logical processor count here, but can't use 1 either because JIT optimization ruins result.
            int logicalProcessorCount = 2;
#if DEBUG
            Console.WriteLine("SingleThreaded calculations started.");
#endif
            sw.Start();

            long resultInt = 0;
            for (int i = 0; i < logicalProcessorCount / 2; i++)
            {
                resultInt = PerformIntegerCalculations(count);
            }

            double resultDouble = 0.0;
            for (int i = 0; i < logicalProcessorCount / 2; i++)
            {
                resultDouble = PerformFloatCalculations(count);
            }

            sw.Stop();
#if DEBUG
            Console.WriteLine($"{count * logicalProcessorCount} amount of SingleThreaded calculations completed in {sw.ElapsedMilliseconds} milliseconds.");
            Console.WriteLine($"Integer result = {resultInt}");
            Console.WriteLine($"Float result = {(long)resultDouble}");
#endif
            long score = (count * logicalProcessorCount) / sw.ElapsedMilliseconds / 10000;
            Console.WriteLine($"SingleThread Score: {score}");

            return score;
        }

        static long MultiThreadedTest(long count)
        {
            Stopwatch sw = new Stopwatch();

            int logicalProcessorCount = Environment.ProcessorCount * 2;
#if DEBUG
            Console.WriteLine("MultiThreaded calculations started.");
#endif
            sw.Start();

            Task[] tasks = new Task[logicalProcessorCount];

            long resultInt = 0;
            double resultDouble = 0.0;

            for (int i = 0; i < logicalProcessorCount / 2; i++)
            {
                tasks[i] = Task.Run(() => resultInt = PerformIntegerCalculations(count));
                tasks[i + logicalProcessorCount / 2] = Task.Run(() => resultDouble = PerformFloatCalculations(count));
            }

            Task.WhenAll(tasks).Wait();

            sw.Stop();
#if DEBUG
            Console.WriteLine($"{count * logicalProcessorCount} amount of MultiThreaded calculations completed in {sw.ElapsedMilliseconds} milliseconds.");
            Console.WriteLine($"Integer result = {resultInt}");
            Console.WriteLine($"Float result = {(long)resultDouble}");
#endif
            long score = (count * logicalProcessorCount) / sw.ElapsedMilliseconds / 10000;
            Console.WriteLine($"MultiThread Score: {score}");

            return score;
        }

        //These two methods are balanced in terms of operational complexity while ensuring same result, floating point errors notwithstanding.
        static long PerformIntegerCalculations(long count)
        {
            long result = 0;
            for (long i = 0; i < count; i++)
            {
                result += i * (1 + 2 + 3 + 4);  // Integer operations
            }
            return result;
        }
        static double PerformFloatCalculations(long count)
        {
            double result = 0.0;
            for (long i = 0; i < count; i++)
            {
                result += i * 10;  // Floating-point operations
            }
            return result;
        }
    }
}