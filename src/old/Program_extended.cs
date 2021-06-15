using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace Magnetron_Deconvolution
{
    class Program
    {
        static void Main(string[] args)
        {
            /*
            Written by Hiroto Motoyama (2017/05/18)
            Modified by Takenori Shimamura (June 13, 2017)
            
            This program is for the NC deposition figuring of KB mirror.
            Only 1 dimensional data can be handled.
            Purpose shape(error.txt) and spot deposition mark(spot.txt) should be placed at the "/bin/debug/input/" directory. 
            See "error_sample.txt" and "spot_sample.txt" to check the data format.
            If calculation loop does not converge in practical time, modify the parameters "alpha" declared just below this message.
            (Value of "alpha" is automatically modified during itterative calculation.)  
            "threshold" and "LOOP" should be also chosen according to your need.
            Calculation results are output to the "/bin/debug/output/results.txt".
            If you want to calculate in plane condition, you manually modify "//break;" →"break;"
            Enjoy deconvolution !
            */
            System.Diagnostics.Stopwatch sw = System.Diagnostics.Stopwatch.StartNew();

            ////////////////////////////////
            //ユーザー調整パラメータここから
            ////////////////////////////////
            //update処理調整用パラメータ、収束しない場合はこの値を調整する

            double alpha = 50;

            //RMSのループ脱出基準、形状誤差のRMS値がこの値より小さくなったら終了する
            double threshold = 0.1;
            int REFRESHTHRESHOLD = 0; // when this is 1, refresh the threshold if RMS becomes smaller than the threshold 
			double DECREMENT = 0.01;
            double EPSILON = 0.001;
            double ALPHABOOST = 0.0001;
            double TRIALS = 0;

            //ループ回数のループ脱出基準、ループ回数がこの値よりも大きくなったら終了する
            int LOOP = 150000;
            //////////
            //ここまで
            //////////

            //デコンボリューション計算用の配列宣言
            double[] purpose = new double[1000000]; //目標加工量分布
            double[] shape = new double[1000000];   //加工量
            double[] error = new double[1000000];   //偏差量
            double[] spot = new double[1000000];    //スポット加工痕
            double[] time = new double[1000000];    //滞在時間分布
            double[] update = new double[1000000];  //滞在時間分布更新用配列(誤差×スポット)

			// for recording the results
			double[] p_result = new double[1000000]; //目標加工量分布
			double[] shape_result = new double[1000000];   //加工量
			double[] e_result = new double[1000000];   //偏差量
			double[] spot_result = new double[1000000];    //スポット加工痕
			double[] t_result = new double[1000000];    //滞在時間分布
			double[] u_result = new double[1000000];  //滞在時間分布更新用配列(誤差×スポット)


			int p_N = 500;  //目標加工量分布のサフィックス最大値(要素数が10なら9)
            int s_N = 100;  //スポット加工量のサフィックス最大値(要素数が奇数である必要、s_Nは偶数)
            int s_n = 0;    //s_Nの半分、デコンボリューションの計算用
            int modified_p_N = p_N + s_N * 2; // enlarge the area that is processed 
            
            //計算に使う各種変数
            int count = 0;
            int count2 = 0;
            int count_pursue = 0;
            int N = 0;
            double RMS = 1000000;
            double rms = 0;

            //スポット加工痕読み込み
            StreamReader dReader = (new StreamReader("input/spot.txt", Encoding.Default));
            count = 0;
            while (dReader.Peek() >= 0)
            {
                string stBuffer = dReader.ReadLine();
                spot[count] = double.Parse(stBuffer);
                count++;
            }
            s_N = count - 1;
            Console.WriteLine("Program: spot size data number s_N = " + s_N);

            //スポットの要素数が偶数の場合、端っこの1要素を削る
            if (s_N % 2 == 0)
            {
                s_N = s_N - 1;
            }
            s_n = s_N / 2;
            dReader.Close();

            //目標加工量読み込み
            StreamReader cReader = (new StreamReader("input/error.txt", Encoding.Default));
            count = 0;
            while (cReader.Peek() >= 0)
            {
                string stBuffer = cReader.ReadLine();
                purpose[count] = double.Parse(stBuffer);
                count++;
            }
            p_N = count - 1;
            Console.WriteLine("Program: the expected shape data number p_N = " + p_N);
            cReader.Close();

            // enlarge the area that is processed
            modified_p_N = p_N + (s_N + 1) * 2;

            for (int i = modified_p_N; i >= 0; i--) {
                if (i >= p_N + s_N + 1) {
                    purpose[i] = purpose[p_N];
                    //Console.WriteLine("purpose  larger = " + purpose[p_N]);
                } else if (i <= s_N) {
					purpose[i] = purpose[s_N + 1];
					//Console.WriteLine("purpose  smaller = " + purpose[p_N]);
                } else {
                    purpose[i] = purpose[i - s_N - 1];
                }
            }

            //滞在時間分布初期化
            for (int i = 0; i <= modified_p_N; i++)
            {
                time[i] = 1;
            }

            ////////////////////
            //ここからループ処理
            ////////////////////
            while (true)
            {
                //ループ回数を計算
                count2++;

                //配列の初期化
                for (int i = 0; i <= modified_p_N; i++)
                {
                    shape[i] = 0;
                    update[i] = 0;
                }

                //加工量算出(滞在時間分布×スポット加工痕)
                //例外処理のコメントアウトを消すと、平面の計算になる
                for (int i = 0; i <= modified_p_N; i++)
                {
                    for (int j = 0; j <= s_N; j++)
                    {
                        N = i - s_n + j;
                        if (N < 0)
                        {
                            N = N + modified_p_N + 1;
                            break; // ここ
                        }
                        else if (N > modified_p_N)
                        {
                            N = N - modified_p_N - 1;
                            break; // ここ
                        }
                        shape[N] = shape[N] + time[i] * spot[j];
                    }
                }

                //誤差分布算出
                rms = RMS;  //1回前のRMS値を保持
                RMS = 0;
                count = 0;
                for (int i = 0; i <= modified_p_N; i++)
                {
                    error[i] = 0;
                    if (s_N + 1 <= i && i <= s_N + p_N + 1)
                    {
                        error[i] = purpose[i] - shape[i];
                        RMS = RMS + error[i] * error[i];
                        count++;
                    }
                }
                RMS = Math.Sqrt(RMS / count);

                //Console.WriteLine(RMS);

                //RMSの値を比較してalphaを調整。滞在時間分布の更新は行わない
                if (rms < RMS)
                {
                    alpha = 0.95 * alpha;
                    Console.WriteLine("Program extended: RMS = " + RMS + ", alpha refreshed less!!");

                    goto LOOPEND;
                }
                if (RMS < 4.0 && RMS - rms < ALPHABOOST && count2 >= TRIALS + 50 ){
                    alpha *= 1.05;
					Console.WriteLine("Program extended: RMS = " + RMS + ", alpha refreshed more!!");
                    TRIALS = count2;
                }
                

                //RMSの値でループ脱出評価
                if (RMS < threshold)
                {
                    threshold-=DECREMENT;
                    if (RMS - DECREMENT < EPSILON) break; 
					p_result = purpose; //目標加工量分布
                    shape_result = shape;   //加工量
					e_result = error;   //偏差量
					spot_result = spot;    //スポット加工痕
					t_result = time;    //滞在時間分布
                    u_result = update; //滞在時間分布更新用配列(誤差×スポット)
                    if (REFRESHTHRESHOLD == 0) break;
                    Console.WriteLine("Program extended: threshold refleshed!! threshold = " + threshold + " for " + count2 + "times");

                }

                //ループ回数で脱出評価
                if (count2 > LOOP)
                {
                    p_result = purpose; //目標加工量分布
                    shape_result = shape;   //加工量
                    e_result = error;   //偏差量
                    spot_result = spot;    //スポット加工痕
                    t_result = time;    //滞在時間分布
                    u_result = update; //滞在時間分布更新用配列(誤差×スポット)
                    Console.WriteLine("Program extended: reach the maximum of loops for " + count2 + " times");
                    break;
                }

                //誤差×スポット算出(誤差分布×スポット加工量)
                for (int i = s_n + 1; i <= modified_p_N + 1 - s_n; i++)
                {
                    for (int j = 0; j <= s_N; j++)
                    {
                        N = i - s_n + j;
                        update[N] = update[N] + error[i] * spot[j]; // why?
                    }
                }

                //滞在時間分布の更新(t=t-α×(p-f))
                for (int i = 0; i <= modified_p_N; i++)
                {
                    time[i] = time[i] + alpha * update[i];
                }

                //非負制限
                for (int i = 0; i <= modified_p_N; i++)
                {
                    if (time[i] < 1)
                    {
                        time[i] = 1;
                        //Console.WriteLine("Program extended: The result may be invalid since the time is less than 1.");
                    }
                }

                LOOPEND:;
                if (count2 % 5000 == 0) {
                    Console.WriteLine("Program extended: count = " + count2 + " in loop, RMS = " + RMS);
                }
				Console.WriteLine("Program extended: RMS = " + RMS + "count," + count2 + ", alpha = " + alpha);




			}
            Console.WriteLine(RMS);
            ////////////////////
            //ここまでループ処理
            ////////////////////

            //計算結果出力
            StreamWriter sw1 = new StreamWriter("output/extension_results.csv");
            DateTime dtNow = DateTime.Now;
            sw1.WriteLine("RMS," + RMS + ",Time," + dtNow.ToString()); 
            sw1.WriteLine("count" + "," + "purpose" + "," + "ResidualError" + "," + "TimeDistribution" + "," + "Working" + "," + "spot");
            for (int i = 0; i <= modified_p_N; i++)
            {
                if (i <= s_N)
                {

                    sw1.WriteLine(i + "," + p_result[i] + "," + e_result[i] + "," + t_result[i] + "," + shape_result[i] + "," + spot_result[i]);
                }
                else
                {
                    sw1.WriteLine(i + "," + p_result[i] + "," + e_result[i] + "," + t_result[i] + "," + shape_result[i] + "," + "0");
                }
            }
            sw1.Close();

			StreamWriter sw2 = new StreamWriter("output/extension_extracted.csv");
			sw2.WriteLine("RMS," + RMS + ",Time," + dtNow.ToString());
            sw2.WriteLine("count" + "," + "purpose" + "," + "ResidualError" + "," + "TimeDistribution" + "," + "Working" + "," + "spot");
            for (int i = s_N + 1; i <= s_N + p_N + 1; i++)
			{
                if (i - s_N <= s_N)
				{

                    sw2.WriteLine(i + "," + p_result[i] + "," + e_result[i] + "," + t_result[i] + "," + shape_result[i] + "," + spot_result[i - s_N]);
				}
				else
				{
					sw2.WriteLine(i + "," + p_result[i] + "," + e_result[i] + "," + t_result[i] + "," + shape_result[i] + "," + "0");
				}
			}
			sw2.Close();
			sw.Stop();
			Console.WriteLine("Elapsed time:D " + sw.Elapsed);

		       }
    }
}

