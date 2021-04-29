//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;
//using System.IO;

//namespace Rotation_Deconvolution
//{
//    class Program
//    {
//        static void Main(string[] args)
//        {
//            /*
//            Written by Hiroto Motoyama (2017/05/18)
//            This program is for the NC deposition figuring of ellipsoidal mirror.
//            Only 1 dimensional data can be handled.
//            Purpose shape(error.txt) and spot deposition mark(spot.txt) should be placed at the "/bin/debug/input/" directory. 
//            See "error_sample.txt" and "spot_sample.txt" to check the data format.
//            If calculation loop does not converge in practical time, modify the parameters "alpha" declared just below this message.
//            (Value of "alpha" is automatically modified during itterative calculation.)  
//            "threshold" and "LOOP" should be also chosen according to your need.
//            Calculation results are output to the "/bin/debug/output/results.txt".
//            If you want to calculate in plane condition, you manually modify "//break;" →"break;"
//            Enjoy deconolution !
//            */
            
//            ////////////////////////////////
//            //ユーザー調整パラメータここから
//            ////////////////////////////////
//            //update処理調整用パラメータ、収束しない場合はこの値を調整する
//            double alpha = 1;

//			//RMSのループ脱出基準、形状誤差のRMS値がこの値より小さくなったら終了する
//			double threshold = 9.0;
//			double DECREMENT = 0.5;
//			double EPSILON = 0.001;

//			//ループ回数のループ脱出基準、ループ回数がこの値よりも大きくなったら終了する
//			int LOOP = 5000;
//            //////////
//            //ここまで
//            //////////

//            //デコンボリューション計算用の配列宣言
//            double[] purpose = new double[1000000]; //目標加工量分布
//            double[] shape = new double[1000000];   //加工量
//            double[] error = new double[1000000];   //加工量
//            double[] spot = new double[1000000];    //スポット加工痕
//            double[] time = new double[1000000];    //滞在時間分布
//            double[] update = new double[1000000];  //滞在時間分布更新用配列(誤差×スポット)

//			// for recording the results
//			double[] p_result = new double[1000000]; //目標加工量分布
//			double[] shape_result = new double[1000000];   //加工量
//			double[] e_result = new double[1000000];   //偏差量
//			double[] spot_result = new double[1000000];    //スポット加工痕
//			double[] t_result = new double[1000000];    //滞在時間分布
//			double[] u_result = new double[1000000];  //滞在時間分布更新用配列(誤差×スポット)


//			int p_N = 500;  //目標加工量分布のサフィックス最大値(要素数が10なら9)
//            int s_N = 100;  //スポット加工量のサフィックス最大値(要素数が奇数である必要、s_Nは偶数)
//            int s_n = 0;    //s_Nの半分、デコンボリューションの計算用
            
//            //計算に使う各種変数
//            int count = 0;
//            int count2 = 0;
//            int count_pursue = 0;
//            int N = 0;
//            double RMS = 0;
//            double rms = 0;

//            //スポット加工痕読み込み
//            StreamReader dReader = (new StreamReader("input/spot.txt", Encoding.Default));
//            count = 0;
//            while (dReader.Peek() >= 0)
//            {
//                string stBuffer = dReader.ReadLine();
//                spot[count] = double.Parse(stBuffer);
//                count++;
//            }
//            s_N = count - 1;

//            //スポットの要素数が偶数の場合、端っこの1要素を削る
//            if (s_N % 2 == 0)
//            {
//                s_N = s_N - 1;

//            }
//            s_n = s_N / 2;
//            dReader.Close();

//            //目標加工量読み込み
//            StreamReader cReader = (new StreamReader("input/error.txt", Encoding.Default));
//            count = 0;
//            while (cReader.Peek() >= 0)
//            {
//                string stBuffer = cReader.ReadLine();
//                purpose[count] = double.Parse(stBuffer);
//                count++;
//            }
//            p_N = count - 1;
//            cReader.Close();

//            //滞在時間分布初期化
//            for (int i = 0; i <= p_N; i++)
//            {
//                time[i] = 1;
//            }

//            ////////////////////
//            //ここからループ処理
//            ////////////////////
//            while (true)
//            {
//                //ループ回数を計算
//                count2++;

//                //配列の初期化
//                for (int i = 0; i <= p_N; i++)
//                {
//                    shape[i] = 0;
//                    update[i] = 0;
//                }

//                //加工量算出(滞在時間分布×スポット加工痕)
//                //例外処理のコメントアウトを消すと、平面の計算になる
//                for (int i = 0; i <= p_N; i++)
//                {
//                    for (int j = 0; j <= s_N; j++)
//                    {
//                        N = i - s_n + j;
//                        if (N < 0)
//                        {
//                            N = N + p_N + 1;
//                            break;
//                        }
//                        else if (N > p_N)
//                        {
//                            N = N - p_N - 1;
//                            break;
//                        }
//                        shape[N] = shape[N] + time[i] * spot[j];
//                    }
//                }

//                //誤差分布算出
//                rms = RMS;  //1回前のRMS値を保持
//                RMS = 0;
//                count = 0;
//                for (int i = 0; i <= p_N; i++)
//                {
//                    error[i] = purpose[i] - shape[i];
//                    RMS = RMS + error[i] * error[i];
//                    count++;
//                }
//                RMS = Math.Sqrt(RMS / count);

//				//Console.WriteLine(RMS);

//				//RMSの値を比較してalphaを調整。滞在時間分布の更新は行わない
//				if (rms < RMS)
//				{
//					alpha = 0.95 * alpha;
//					Console.WriteLine("Program extended: RMS = " + RMS);
//					count_pursue++;
//					goto LOOPEND;
//				}


//                //RMSの値でループ脱出評価
//                if (RMS < threshold)
//                {
//                    threshold -= DECREMENT;
//                    if (RMS - DECREMENT < EPSILON) break;
//                    p_result = purpose; //目標加工量分布
//                    shape_result = shape;   //加工量
//                    e_result = error;   //偏差量
//                    spot_result = spot;    //スポット加工痕
//                    t_result = time;    //滞在時間分布
//                    u_result = update; //滞在時間分布更新用配列(誤差×スポット)
//                    Console.WriteLine("Program extended: threshold refleshed!! threshold = " + threshold + " for " + count2 + " times");
//                }

//					//ループ回数で脱出評価
//					if (count2 > LOOP)
//                {
//					p_result = purpose; //目標加工量分布
//					shape_result = shape;   //加工量
//					e_result = error;   //偏差量
//					spot_result = spot;    //スポット加工痕
//					t_result = time;    //滞在時間分布
//					u_result = update; //滞在時間分布更新用配列(誤差×スポット)
//					Console.WriteLine("Program extended: reach the maximum of loops for " + count2 + " times");
//                    break;
//                }

//                //誤差×スポット算出(誤差分布×スポット加工量)
//                for (int i = 0; i <= p_N; i++)
//                {
//                    for (int j = 0; j <= s_N; j++)
//                    {
//                        N = i - s_n + j;
//                        if (N < 0)
//                        {
//                            N = N + p_N + 1;
//                            break;
//                        }
//                        else if (N > p_N)
//                        {
//                            N = N - p_N - 1;
//                            break;
//                        }
//                        update[N] = update[N] + error[i] * spot[j];
//                    }
//                }

//                //滞在時間分布の更新(t=t-α×(p-f))
//                for (int i = 0; i <= p_N; i++)
//                {
//                    time[i] = time[i] + alpha * update[i];
//                }

//                //非負制限
//                for (int i = 0; i <= p_N; i++)
//                {
//                    if (time[i] < 0)
//                    {
//                        time[i] = 0;
//                    }
//                }

//                LOOPEND:;
                
//            }
//            Console.WriteLine(RMS);
//            ////////////////////
//            //ここまでループ処理
//            ////////////////////

//            //計算結果出力
//            StreamWriter sw1 = new StreamWriter("output/original_results.csv");
//            sw1.WriteLine("RMS," + RMS);
//            sw1.WriteLine("count" + "," + "purpose" + "," + "ResidualError" + "," + "TimeDistribution" + "," + "Working" + "," + "spot");
//            for (int i = 0; i <= p_N; i++)
//            {
//                if (i <= s_N)
//                {

//                    sw1.WriteLine(i + "," + p_result[i] + "," + e_result[i] + "," + t_result[i] + "," + shape_result[i] + "," + spot_result[i]);
//                }
//                else
//                {
//                    sw1.WriteLine(i + "," + p_result[i] + "," + e_result[i] + "," + t_result[i] + "," + shape_result[i] + "," + "0");
//                }
//            }
//            sw1.Close();
            
//        }
//    }
//}
