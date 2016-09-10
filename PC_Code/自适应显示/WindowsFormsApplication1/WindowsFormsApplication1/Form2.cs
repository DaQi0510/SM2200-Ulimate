using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace WindowsFormsApplication1
{
    public partial class Form2 : Form
    {
        public Form2()
        {
            InitializeComponent();
            for (byte i = 1; i < 9; i++)
            {
                if(i!=G.ConnectDevice)
                    comboBox1.Items.Add(i.ToString());
            }
            for (byte i = 1; i < 9; i++)
            {
                if (i != G.ConnectDevice)
                {
                    comboBox1.Text = i.ToString();
                    break;
                }
            }
            comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 0;
            G.FScale = 1;
            G.ConnectToDevice = 1;
            G.StartVoltage = 4;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            this.Dispose();
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            G.StartVoltage = (byte)int.Parse(textBox1.Text.Trim());
        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            G.ConnectToDevice =(byte )comboBox1.SelectedIndex;
            G.ConnectToDevice +=1;
            if (G.ConnectToDevice >= G.ConnectDevice)
                G.ConnectToDevice += 1;
        }

        private void comboBox2_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (comboBox2.SelectedIndex == 0)   //主模式
            {
                comboBox1.Enabled = true ;
                G.FScale = 1;
            }
            else
            {
                comboBox1.Enabled = false;
                G.FScale = 0;
            }
        }
    }
}
