Running web documents in Apache.

Apache has an 'htdocs' directory where web files are kept & served. Copy web files *.html or *.php files into this directory.

Install PHP (per online instructions for linux).

MySQL. (Install MySQL per online instructions for linux). There should be scripts to generate tables, but until that's done, there are only
two tables used by the web docs.

The username/password occurs frequently in the webdocs & is specified any time a transaction is done with the database. User 'xvdth' &
password '12345'.

database name: xvdth

table 'user'

+----+-------+-------+----------+----------+------------------------------------+--------+-----------+-----------+-------+--------+
| id | first | last  | username | password | ip                                 | online | insession | colchange | width | height |
+----+-------+-------+----------+----------+------------------------------------+--------+-----------+-----------+-------+--------+
|  1 | Sue   | Smith | Sue      | password | rtsp://::1:8554/stream0            |      1 |         1 |         0 |   640 |    360 |
|  2 | Eve   | Smith | Eve      | password | rtsp://192.168.46.129:8554/stream0 |      0 |         0 |         0 |   640 |    360 |
|  3 | Joe   | Smith | Joe      | password | rtsp://192.168.46.81:8554/stream0  |      0 |         0 |         0 |   640 |    720 |
|  4 | Tim   | Smith | Tim      | password | rtsp://192.168.46.81:8554/stream0  |      0 |         0 |         0 |   640 |    360 |
+----+-------+-------+----------+----------+------------------------------------+--------+-----------+-----------+-------+--------+

pretty straight forward. The 'colchange' column is not used.

table 'sessioncount' - count of users in session.

+-------+
| count |
+-------+
|     0 |
+-------+




