#include "framework.h"
#include "iRecHS2s.h"



enum class tcpip_state {
    listen_state,receive_state,
};

iRec::net_io::udp_int::udp_int(net_io* nio, boost::asio::io_context& io, boost::asio::ip::port_type port)
    :nio(nio), io(io), port_num(port), sok(io, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port))
{

    wait_for_receive();
}
void iRec::net_io::udp_int::wait_for_receive()
{
    sok.async_receive_from(
        boost::asio::buffer(recv_int), remote_endpoint,
        boost::bind(&udp_int::receive_int, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void iRec::net_io::udp_int::receive_int(const boost::system::error_code& error, std::size_t siz)
{
    if (!error)
    {
        std::string adr, msg,port;
        UdpIntData uid;
        int a = recv_int[0];
        boost::endian::big_to_native_inplace(a);
        adr = remote_endpoint.address().to_string();
        //port = (boost::format("%d") % remote_endpoint.port()).str();
        port = (boost::format("%d") % port_num).str();
        msg = (boost::format("%s:%d") % adr% a).str();

        uid.data = a;
        QueryPerformanceCounter(&uid.FreqCount);
        uid.ti =(double) (uid.FreqCount.QuadPart - nio->iRecPtr->CPU_StartFreqCount.QuadPart)/(double)nio->iRecPtr->CPU_Freq.QuadPart;
        uid.fromAddress = adr;
        uid.port = port;
        v.push_back(uid);


        nio->printmsg(NetProc::udpint, msg);
        wait_for_receive();
    }
}

iRec::net_io::udp_int::~udp_int()
{

}

iRec::net_io::udp_str::udp_str(net_io* nio, boost::asio::io_context& io_context, boost::asio::ip::port_type port)
    : nio(nio), io(io_context), port_num(port), sok(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port))
{
    wait_for_receive();
}

void iRec::net_io::udp_str::wait_for_receive()
{
    sok.async_receive_from(boost::asio::buffer(recv_str), remote_endpoint,
        boost::bind(&udp_str::receive_str,
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void iRec::net_io::udp_str::receive_str(const boost::system::error_code& error, std::size_t siz)
{
    if (!error)
    {
        std::string str, adr, msg;
        UdpStrData usd;
      

        adr = remote_endpoint.address().to_string();
        str = std::string(static_cast<const char*>(recv_str.data()), siz);
        msg = (boost::format("%s:%s") % adr % str).str();
        nio->printmsg(NetProc::udpstr,msg);

        QueryPerformanceCounter(&usd.FreqCount);
        usd.ti = (double)(usd.FreqCount.QuadPart - nio->iRecPtr->CPU_StartFreqCount.QuadPart) / (double)nio->iRecPtr->CPU_Freq.QuadPart;
        usd.fromAddress = adr;
        usd.msg = str;
        usd.port = (boost::format("%d") % port_num).str();
        v.push_back(usd);
        wait_for_receive();
    }
}

iRec::net_io::udp_str::~udp_str()
{

}


void iRec::net_io::tcp_io::send(std::string& str)
{
    if (sok == NULL) return;
    if (sok->is_open()) {
        boost::asio::write(*sok, boost::asio::buffer(str));
    }
}
void iRec::net_io::tcp_io::async_send(std::string& str)
{
    if (sok == NULL) return;
    if (sok->is_open()) {
        boost::asio::async_write(*sok, boost::asio::buffer(str), bind(&tcp_io::async_send_result, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
}
void iRec::net_io::tcp_io::async_send_result(const boost::system::error_code& e, size_t siz)
{
    if (e) {
        //cout << "write failed: " << e.message() << endl;
    }
}
void iRec::net_io::tcp_io::listen()
{
    std::string str = (boost::format("Waiting for client (%d)") % port).str();
    nio->printmsg(NetProc::tcpip,str);

    IsSend = false;
    if (sok != NULL) delete sok;
    if (acc != NULL) delete acc;

    buf.consume(buf.size());
    sok = new boost::asio::ip::tcp::socket(io);
    acc = new boost::asio::ip::tcp::acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
    acc->async_accept(*sok, bind(&tcp_io::accept, this, boost::placeholders::_1));
}

void iRec::net_io::tcp_io::accept(const boost::system::error_code& e)
{
    delete acc;
    std::string msg;
    acc = NULL;
    if (e) {
        listen();
        return;
    }
    sok->set_option(boost::asio::ip::tcp::no_delay(true));
    //address = sok->remote_endpoint().address().to_string();
    //msg = (boost::format("Remote IP address: %s") % address).str();
    //nio->printmsg(NetProc::tcpip, msg);

    receive();
}
void iRec::net_io::tcp_io::receive()
{
    address = sok->remote_endpoint().address().to_string();
    std:: string msg = (boost::format("Connect: %s") % address).str();
    nio->printmsg(NetProc::tcpip,msg);

    async_read_until(*sok, buf, '\n',
        boost::bind(&tcp_io::interprete, this,
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void iRec::net_io::tcp_io::interprete(const boost::system::error_code& e, size_t siz)
{
    std::vector<std::string> vec;
    std::string msg = "";
    boost::char_separator<char> sep(" ,\n");
    TcpCalData td;
    std::wstring ws;
    if (e) {
        //cout << "command failed: " << e.message() << endl;
        listen();
        return;
    }

    const std::string str(boost::asio::buffer_cast<const char*>(buf.data()), siz);
    buf.consume(siz);

    boost::tokenizer< boost::char_separator<char>> tokens(str, sep);
    for (auto it = tokens.begin(); it != tokens.end(); ++it) {
        vec.push_back(*it);
    }

    if (!vec.empty()) {
        if (vec[0] == "disconnect") {
            listen();
            return;
        }
        if (vec[0] == "start") {
            IsSend = true;
            msg = "Command: start";
        }
        if (vec[0] == "stop") {
            IsSend = false;
            msg = "Command: stop";
        }
        if (vec[0] == "calibration") {
            if (vec.size() == 5) {
                td.sti = std::stod(vec[1]);
                td.h = std::stod(vec[2]);
                td.v = std::stod(vec[3]);
                td.s = std::stod(vec[4]);
                QueryPerformanceCounter(&td.FreqCount);
                td.ti = (double)(td.FreqCount.QuadPart - nio->iRecPtr->CPU_StartFreqCount.QuadPart) / (double)nio->iRecPtr->CPU_Freq.QuadPart;
                td.fromAddress = address;
                td.port= (boost::format("%d") % port).str();
                v.push_back(td);
                msg = (boost::format("calibration  %f, %f, %f, %f") % td.sti % td.h % td.v % td.s).str();
            }
        }
        if (vec[0] == "hello") {
            msg = "hello";
            std::string ret_msg = "world";
            async_send(ret_msg);
        }
        if (msg == "") {
            msg = "Unknown command: " + vec[0];
        }
        if (msg != "") {
            //nio->printmsg(NetProc::tcpip, msg);
        }
    }
    receive();
}
iRec::net_io::tcp_io::tcp_io(net_io* nio, boost::asio::io_context& io, boost::asio::ip::port_type port_num)
    :nio(nio), io(io), port(port_num), sok(NULL), acc(NULL)
{

    IsSend = false;
    listen();
}


iRec::net_io::tcp_io::~tcp_io()
{
    buf.consume(buf.size());
    if (sok != NULL) {
        delete sok;
    }
    if (acc != NULL) {
        delete acc;
    }
}


iRec::net_io::net_io(iRec& ir, boost::asio::ip::port_type tcp_port, boost::asio::ip::port_type udp_int_port, boost::asio::ip::port_type udp_str_port )
    :iRecPtr(&ir),
    t_server(this, io, tcp_port),
    ui_server(this, io, udp_int_port),
    us_server(this, io, udp_str_port),
    th(boost::bind(&boost::asio::io_context::run, &io)),
    hWnd(NULL)
{
  
    //th = new boost::thread(boost::bind(&boost::asio::io_context::run, &io));
}


iRec::net_io::~net_io()
{
    hWnd = NULL;
    io.stop();
    th.join();
}

static std::string wstr2str(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}
static std::wstring str2wstr(const std::string& str)
{
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}


void iRec::net_io::printmsg(NetProc np,std::string msg)
{
    std::wstring ws;
    ws = str2wstr(msg);
    //MessageBox(NULL, ws.c_str(),
    //    TEXT("メッセージボックス"),
    //    MB_OK | MB_ICONINFORMATION);
    if (hWnd == NULL) return;

    switch (np) {
    case NetProc::tcpip:
        SendMessage(hWnd, SB_SETTEXT, 0 | 0, (LPARAM)ws.c_str());
        break;
    case NetProc::udpint:
        SendMessage(hWnd, SB_SETTEXT, 1 | 0, (LPARAM)ws.c_str());
        break;
    case NetProc::udpstr:
        SendMessage(hWnd, SB_SETTEXT, 2 | 0, (LPARAM)ws.c_str());
        break;
    }
}

void iRec::net_io::SetStatusWindow(HWND wh)
{
    std::wstring ws;
    std::string str;
    hWnd = wh;
    str = (boost::format("tcp:%d") % t_server.port).str();
    ws = str2wstr(str);
    SendMessage(hWnd, SB_SETTEXT, 0 | 0, (LPARAM)ws.c_str());

    str = (boost::format("udp integer:%d") % ui_server.port_num).str();
    ws = str2wstr(str);
    SendMessage(hWnd, SB_SETTEXT, 1 | 0, (LPARAM)ws.c_str());

    str = (boost::format("udp string:%d") % us_server.port_num).str();
    ws = str2wstr(str);
    SendMessage(hWnd, SB_SETTEXT, 2 | 0, (LPARAM)ws.c_str());
}

bool iRec::net_io::SendData(EyeNumData& ed)
{
    double ti;
    std::string str;
    if (t_server.IsSend == false) return false;


    ti = fabs(ed.TimeStamp - iRecPtr->CAM_StartTimeStamp) / 1e9;
    str = (boost::format("%8.3f,%6.2f,%6.2f,%6.2f,%6.2f\n") % ti % ed.Gaze.x % ed.Gaze.y % ed.Gaze.l % ed.Gaze.openratio).str();
    t_server.async_send(str);
    //t_server.send(str);
    return true;
    
}