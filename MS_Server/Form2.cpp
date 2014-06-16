// ============== Schwartsune Kumiko 2014 =========================
//
// : Master Server Server
//
// ================================================================
#include "Form2.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThread]
void Main(array<String^>^ args)
{
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(true);

	MS_Server::Form2 form;
	Application::Run(%form);
}


// TODO (Schwartsune Kumiko): ����� ������-������� � ���-��������: 
// ��� �������� -> ���������� ���������� � ������-�������� � ������� IP ������.
// ��� ���������� -> ������� ������ �������� � IP �������.
void MS_Server::Form2::WhenClosed(Object^ sender, FormClosedEventArgs^ e)
{
	server->Stop();
	WaitingForClients = false;
}
void MS_Server::Form2::WhenClosing(Object^ sender, FormClosingEventArgs^ e)
{
	// for losers
}
void MS_Server::Form2::WhenLoad(Object^ sender, EventArgs^ e)
{
	try
	{
		server = gcnew TcpListener(27016); // ���� ������ ���� 228
		server->Start();
		log->AppendText("Master Server succefully started.\n");
		WaitingForClients = true;
		Thread^ acceptClientThread = gcnew Thread(gcnew ThreadStart(this, &MS_Server::Form2::AcceptClients));
		acceptClientThread->Start();
	}
	catch (SocketException^ exp)
	{
		MessageBox::Show(Convert::ToString(exp));
	}
}
void MS_Server::Form2::AcceptClients()
{
	while (WaitingForClients) // � ����� ����� �������
	{
		try // ���� ���� ������
		{
			TcpClient^ user = server->AcceptTcpClient();
			Thread^ clientThread = gcnew Thread(gcnew ParameterizedThreadStart(this, &MS_Server::Form2::ClientThread));
			clientThread->Start(user);
			
		}
		catch (Exception^ exp) // �� �����
		{
			break;
		}
	}
}
void MS_Server::Form2::ClientThread(Object^ data)
{
	TcpClient ^user = (TcpClient^)data;
	String^ signal; 
	StreamReader^ reader = gcnew StreamReader(user->GetStream()); // ������ ��������
	StreamWriter^ writer = gcnew StreamWriter(user->GetStream());
	writer->AutoFlush = true;
	while (WaitingForClients)
	{
		try
		{
			signal = reader->ReadLine();
			if (signal->IndexOf("REFRESH") > -1) 
			{
				log->AppendText("Sending servers list...\n");
				for each(Object^ obj in ipList)
				{
					String^ ip = (String^)obj;
					writer->WriteLine(ip);
				}
			}
			else if (signal->IndexOf("IPc") > -1)
			{
				String^ ip = signal->Substring(4);
				ipList->Add(ip);
				log->AppendText("A new server has been hosted.\n");
			}
			else if (signal->IndexOf("IPd") > -1)
			{
				String^ ip = signal->Substring(4);
				ipList->RemoveAt(ipList->IndexOf(ip));
				log->AppendText("A server has been disconnected.\n");
			}
		}
		catch (Exception^ exp)
		{
			break; // ������ ���� ������ ���� ���� ������.
		}
	}
}