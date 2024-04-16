#include <inc/core/Core.h>
#include <inc/store/ISQL.h>

#define KCC_FILE    "sql"
#define KCC_VERSION "$Id: sql.cpp 20677 2007-09-12 15:21:44Z tvk $"

void simpletest()
{
    kcc::Log::Scope scope(KCC_FILE, "simpletest");

    kcc::Properties sqlConfig;
    sqlConfig.set("MySql.host",     "localhost");
    sqlConfig.set("MySql.db",       "simpletest");
    sqlConfig.set("MySql.user",     "root");
    sqlConfig.set("MySql.password", "mysql");
    kcc::AutoPtr<kcc::ISQL> sql(KCC_COMPONENT(kcc::ISQL, "k_mysql"));
    if (!sql->init(sqlConfig)) throw kcc::Exception("sql init failed");

    // connection
    kcc::AutoPtr<kcc::ISQLConnection> con(sql->connect());

    // dml test
    con->execute("drop table if exists test");
    con->execute("create table test (num int,str text)"); 

    // auto commit test
    con->execute("insert into test (num,str) values (5150,'uno')");
    con->execute("insert into test (num,str) values (5151,'dos')");

    // check commit
    {
        kcc::SQLFields row(con);
        row.add(kcc::ISQLField::T_STRING);
        kcc::AutoPtr<kcc::ISQLQuery> rs(con->query("select str from test"));
        rs->begin(row);
        while (rs->next()) std::cout << "row=[" << row[0]->toString() << "]" << std::endl;
    }
    
    // bound params query test
    {
        kcc::SQLFields params(con), row(con);
        params.add(kcc::ISQLField::T_LONG);
        row.add(kcc::ISQLField::T_STRING);
        kcc::AutoPtr<kcc::ISQLQuery> rs(con->query("select str from test where num = ?"));
        rs->prepare(row, params);
        params[0]->fLong(5150);
        rs->execute();
        if (rs->next()) std::cout << "bound row=[" << row[0]->fString() << "]" << std::endl;
        params[0]->fLong(5151);
        rs->execute();
        if (rs->next()) std::cout << "bound row=[" << row[0]->fString() << "]" << std::endl;
    }

    // bound params update test
    {
        kcc::SQLFields params(con);
        params.add(kcc::ISQLField::T_LONG);
        kcc::AutoPtr<kcc::ISQLUpdate> up(con->update("delete from test where num = ?"));
        up->prepare(params);
        params[0]->fLong(5150);
        up->execute();
        params[0]->fLong(5151);
        up->execute();
    }

    // check delete
    {
        kcc::SQLFields row(con);
        row.add(kcc::ISQLField::T_STRING);
        kcc::AutoPtr<kcc::ISQLQuery> rs(con->query("select str from test"));
        rs->begin(row);
        if (!rs->next()) std::cout << "bound rows deleted" << std::endl;
    }

    // bound string params insert test
    {
        kcc::SQLFields params(con);
        params.add(kcc::ISQLField::T_LONG);
        params.add(kcc::ISQLField::T_STRING);
        kcc::AutoPtr<kcc::ISQLUpdate> up(con->update("insert into test (num,str) values(?,?)"));
        up->prepare(params);
        params[0]->fLong(2125);
        params[1]->fString("simple text");
        up->execute();
        params[0]->fLong(2127);
        params[1]->fString("crazy text: now\'is?the\"time%for"); // TODO: add auto escape
        up->execute();
    }

    // transaction commit
    con->begin();
    con->execute("insert into test (num,str) values (5152,'abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxy abcdefghijklmnopqrstuvwxy abcdefghijklmnopqrstuvwxy abcdefghijklmnopqrstuvwxy')");
    con->execute("insert into test (num,str) values (5153,NULL)");
    con->commit();

    // transaction rollback
    con->begin();
    con->execute("insert into test (num,str) values (5154,NULL)");
    con->execute("insert into test (num,str) values (5155,NULL)");
    con->rollback();

    // results
    {
        kcc::SQLFields row(con);
        row.add(kcc::ISQLField::T_LONG);
        row.add(kcc::ISQLField::T_STRING);
        kcc::AutoPtr<kcc::ISQLQuery> rs(con->query("select num,str from test"));
        rs->begin(row);
        while (rs->next()) std::cout << "row=[" << row[0]->toString() << ", " << row[1]->toString() << "]" << std::endl;
    }

    // cursor
    {
        // insert several rows
        kcc::SQLFields params(con);
        params.add(kcc::ISQLField::T_LONG);
        params.add(kcc::ISQLField::T_STRING);
        kcc::AutoPtr<kcc::ISQLUpdate> up(con->update("insert into test (num,str) values(?,?)"));
        try
        {
            con->begin();
            up->prepare(params);
            for (int i = 0; i < 500; i++)
            {
                params[0]->fLong(i);
                params[1]->fString(kcc::Strings::printf("data:%d", i));
                up->execute();
            }
            con->commit();
        }
        catch (kcc::SQLException& e)
        {
            con->rollback();
            kcc::Log::exception(e);
        }

        // test query cursor num rows & seek
        kcc::SQLFields row(con);
        row.add(kcc::ISQLField::T_LONG);
        row.add(kcc::ISQLField::T_STRING);
        kcc::AutoPtr<kcc::ISQLQuery> rs(con->query("select num,str from test where num <= 500"));
        rs->begin(row);
        std::cout << "cursor rows: " << rs->rows() << std::endl;
        rs->seek(450);
        while (rs->next()) std::cout << "row=[" << row[0]->toString() << ", " << row[1]->toString() << "]" << std::endl;
    }
}

void fieldtest()
{
    kcc::Log::Scope scope(KCC_FILE, "fieldtest");

    kcc::Properties sqlConfig;
    sqlConfig.set("MySql.host",     "localhost");
    sqlConfig.set("MySql.db",       "fieldtest");
    sqlConfig.set("MySql.user",     "root");
    sqlConfig.set("MySql.password", "mysql");
    kcc::AutoPtr<kcc::ISQL> sql(KCC_COMPONENT(kcc::ISQL, "k_mysql"));
    if (!sql->init(sqlConfig)) throw kcc::Exception("sql init failed");
    kcc::AutoPtr<kcc::ISQLConnection> con(sql->connect());

    con->execute("drop table if exists test");
    con->execute("create table test (sht smallint,lng int,flt float,dbl double,byt tinyint(1) unsigned,str varchar(45),dat date,tim time,dt datetime)");
    con->execute("insert into test (sht,lng,flt,dbl,byt,str,dat,tim,dt) values (5150,16000000,15.50,5150.51505150,225,'now is the time','2006-02-08','18:15:12','2006-02-08 18:15:12')");

    kcc::SQLFields row(con);
    row.add(kcc::ISQLField::T_SHORT);
    row.add(kcc::ISQLField::T_LONG);
    row.add(kcc::ISQLField::T_FLOAT);
    row.add(kcc::ISQLField::T_DOUBLE);
    row.add(kcc::ISQLField::T_BYTE);
    row.add(kcc::ISQLField::T_STRING);
    row.add(kcc::ISQLField::T_DATE);
    row.add(kcc::ISQLField::T_TIME);
    row.add(kcc::ISQLField::T_DATETIME);
    kcc::AutoPtr<kcc::ISQLQuery> rs(con->query("select sht,lng,flt,dbl,byt,str,dat,tim,dt from test"));
    rs->begin(row);
    while (rs->next())
    {
        std::cout << "row=[";
        for (int i = 0; i < row.size(); i++) 
        {
            if (i > 0) std::cout << ", ";
            if (row[i]->type() == kcc::ISQLField::T_BYTE && row[i]->truncated())
                std::cout << (int)((unsigned char)row[i]->fByte()); // test trunc of sign
            else
                std::cout << row[i]->toString();

        }
        std::cout << std::endl;
    }
}

void timetest()
{
    kcc::Log::Scope scope(KCC_FILE, "timetest");

    kcc::Properties sqlConfig;
    sqlConfig.set("MySql.host",     "localhost");
    sqlConfig.set("MySql.db",       "timetest");
    sqlConfig.set("MySql.user",     "root");
    sqlConfig.set("MySql.password", "mysql");
    kcc::AutoPtr<kcc::ISQL> sql(KCC_COMPONENT(kcc::ISQL, "k_mysql"));
    if (!sql->init(sqlConfig)) throw kcc::Exception("sql init failed");
    kcc::AutoPtr<kcc::ISQLConnection> con(sql->connect());

    con->execute("drop table if exists test");
    con->execute("create table test (dt datetime, zone varchar(16))");
    con->execute("insert into test (dt,zone) values (now(),'local')");

    kcc::SQLFields params(con);
    params.add(kcc::ISQLField::T_DATETIME);
    params.add(kcc::ISQLField::T_STRING);
    kcc::AutoPtr<kcc::ISQLUpdate> up(con->update("insert into test (dt,zone) values(?,?)"));
    up->prepare(params);
    params[0]->fDateTime(kcc::SQLDate::utc());
    params[1]->fString("utc");
    up->execute();
    params[0]->fDateTime(kcc::SQLDate::local());
    params[1]->fString("local");
    up->execute();

    // utc to local offset
    std::time_t base = std::time(NULL);
    std::time_t utc2loc = base - std::mktime(std::gmtime(&base));

    kcc::SQLFields row(con);
    row.add(kcc::ISQLField::T_DATETIME);
    row.add(kcc::ISQLField::T_STRING);
    kcc::AutoPtr<kcc::ISQLQuery> rs(con->query("select dt,zone from test"));
    rs->begin(row);
    while (rs->next()) 
    {
        kcc::ISQLField* dt = row[0];
        std::time_t time = dt->fDateTime(); // implicit conversion to time_t
        kcc::String zone = row[1]->fString();
        if (zone == "utc") time += utc2loc;
        std::cout << 
            "field=[" << dt->toString() <<
            "]\tzone=[" << zone <<
            "]\tlocal=[" << kcc::ISODate::local(time) << "]" << std::endl;
    }
}

void createdb()
{
    kcc::Properties sqlConfig;
    sqlConfig.set("MySql.host",     "localhost");
    sqlConfig.set("MySql.db",       "test");
    sqlConfig.set("MySql.user",     "root");
    sqlConfig.set("MySql.password", "mysql");
    kcc::AutoPtr<kcc::ISQL> sql(KCC_COMPONENT(kcc::ISQL, "k_mysql"));
    if (!sql->init(sqlConfig)) throw kcc::Exception("sql init failed");
    kcc::AutoPtr<kcc::ISQLConnection> con(sql->connect());

    con->execute("drop database if exists simpletest");
    con->execute("create database simpletest");

    con->execute("drop database if exists fieldtest");
    con->execute("create database fieldtest");

    con->execute("drop database if exists timetest");
    con->execute("create database timetest");

    con->execute("drop database if exists stringbug");
    con->execute("create database stringbug");
}

void dateinstrinsictest()
{
    kcc::SQLDate d1(2006,11,1,0,0,0);
    kcc::SQLDate d2(2005,11,1,0,0,0);
    std::cout << (d1 < d2) << std::endl;

    kcc::SQLDate* p1 = &d1;
    kcc::SQLDate* p2 = &d2;
    std::cout << (*p1 < *p2) << std::endl;

    typedef std::vector<kcc::SQLDate> DateVector;
    DateVector dv;
    dv.push_back(d1);
    dv.push_back(d2);
    std::sort(dv.begin(), dv.end());
    for (DateVector::iterator i = dv.begin(); i < dv.end(); i++)
        std::cout << *i << std::endl;

    typedef std::map<kcc::SQLDate, kcc::String> DateMap;
    DateMap dm;
    dm[d1] = "06";
    dm[d2] = "05";
    for (DateMap::iterator i = dm.begin(); i != dm.end(); i++)
        std::cout << i->first << ":" << i->second << std::endl;

    typedef std::set<kcc::SQLDate> DateSet;
    DateSet ds;
    ds.insert(d1);
    ds.insert(d2);
    for (DateSet::iterator i = ds.begin(); i != ds.end(); i++)
        std::cout << *i << std::endl;
}

void stringtest()
{
    kcc::Properties sqlConfig;
    sqlConfig.set("MySql.host",     "localhost");
    sqlConfig.set("MySql.db",       "stringbug");
    sqlConfig.set("MySql.user",     "root");
    sqlConfig.set("MySql.password", "mysql");

    kcc::AutoPtr<kcc::ISQL> sql(KCC_COMPONENT(kcc::ISQL, "k_mysql"));
    if (!sql->init(sqlConfig))throw kcc::Exception("sql connection failed");
    kcc::AutoPtr<kcc::ISQLConnection> con(sql->connect());

    con->execute("create table document (md5 varchar(32),content text)");

    kcc::StringMap data;
    data["0102030405060708090a0b0c0d0e0fa0"] = 
        "01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
    data["0102030405060708090a0b0c0d0e0fa1"] =
        "Evening Gown for Formal Night      Bella Online&nbsp;-    Jul 29, 2005     There are of course hundreds of wedding dress and prom dress sites on the web. There is a big problem there - money. These sites    Cadets are indeed the salt of the earth      Buffalo News,&nbsp; United States&nbsp;-    Aug 21, 2005     The selection of her prom dress and graduation dress when each one looked so perfect on her. The holiday dinners, the vacations, the    Husband killed his newlywed wife, police say      Miami Herald,&nbsp;FL&nbsp;-    Aug 17, 2005     ticket her senior year. Instead, she used the money to buy red satin and sew a prom dress for a friend who could not afford one.    Seniors get to be belles, beaux of the ball at Prom      Biloxi Sun Herald,&nbsp; USA&nbsp;-    Aug 20, 2005     really dress up for it,&quot; said Rose Joe, director of community services for Jackson County Civic Action Committee, a co-sponsor. &quot;It&#39;s sort of like the prom    Prom 44: Royal Philharmonic/Gatti, Prom 45: Estonian Philharmonic       Independent,&nbsp;UK&nbsp;-    Aug 19, 2005     Christine Sch fer dressed for the part - first in a revealing corset dress as the The late-night Prom was a guide, homage and progress report on the Estonian    Feminists for (Fetal) Life      AlterNet,&nbsp;CA&nbsp;-    Aug 16, 2005     career&#8211;FFL&#39;s line is thus an advance on the more typical antichoice position, which is that women have abortions to go to Europe or fit into their prom dress.    Lecam: Being marched right back to barefoot and pregnant      Weymouth News,&nbsp;MA&nbsp;-    Aug 16, 2005     baby. The headless womb belonged to a generic woman who, as one opponent said, would get an abortion to fit into a prom dress. She    California drabbing: Hollywood hosts prom dates, superheros      Boston Herald,&nbsp;United States&nbsp;-    Aug 20, 2005     Desperate Housewives&#39;&#39; character is dancing the blanket polka with a teenager, but that&#39;s no " " reason to show up for the &#8220;Teen Choice Awards&#39;&#39; in a prom dress.    This entry was posted on Tuesday, August 23rd, 2005 at 8:34 pmand is filed under   Uncategorized .You can follow any responses to this entry through the   RSS 2.0  feed. You can   leave a response , or   trackback  from your own site.";
    data["0102030405060708090a0b0c0d0e0fa2"] =
	    "The end of oil is closer than you think  |   Home  |   Pingoat - The stable of all pings    August 23, 2005     Start-up drills for oil in algae       With recent news on Peak Oil and all these talk about high gas price. New alternative like GreenFuel is in the pipeline.     Start-up drills for oil in algae | CNET News.com  The potential benefits of the system are twofold: Heavy polluters can cut down on their emissions, and the system can be used for large-scale biodiesel production. Biodiesel, which is often created with vegetable oils from crops such as soybeans, can be used as an alternative to petroleum-based diesel fuel in cars or trucks.       Cat:&nbsp;    Science &nbsp;| Time:&nbsp;9:15 am&nbsp;(UTC+8)&nbsp;         This entry was posted on Tuesday, August 23rd, 2005 at 9:15 am and is filed under   Science . You can follow any responses to this entry through the   RSS 2.0  feed. You can   leave a response , or   trackback  from your own site.           Post your opinion      Name      Mail (will not be published)      Website        ";

    kcc::SQLFields insertParams(con);
    insertParams.add(kcc::ISQLField::T_STRING);
    insertParams.add(kcc::ISQLField::T_STRING);
    kcc::AutoPtr<kcc::ISQLUpdate> insert(con->update("insert into document (md5, content) values (?, ?)"));
    insert->prepare(insertParams);
    for (kcc::StringMap::iterator i = data.begin(); i != data.end(); i++)
    {
        insertParams[0]->fString(i->first);
        insertParams[1]->fString(i->second);
        insert->execute();
    }

    kcc::SQLFields row(con);
    row.add(kcc::ISQLField::T_STRING);
    row.add(kcc::ISQLField::T_STRING);
    kcc::AutoPtr<kcc::ISQLQuery> rs(con->query("select md5,content from document"));
    rs->begin(row);
    while (rs->next())
    {
        kcc::String md5 = row[0]->fString();
        kcc::String txt = row[1]->fString();
        std::cout << "param rebind " << (txt == data[md5] ? "succeeded" : "FAILED") << std::endl;
    }
}

int main(int argc, const char* argv[])
{
    kcc::Properties props;
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    props.set("kcc.logMax",       1L);
    props.set("kcc.LogName",      KCC_FILE);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    kcc::Log::Scope scope(KCC_FILE, "main");
    try
    {
        createdb();
        simpletest();
        fieldtest();
        timetest();
        stringtest();
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}
