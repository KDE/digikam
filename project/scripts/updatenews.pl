use strict;
use LWP::UserAgent;
use File::Slurp;

my $NEWS_FILE = "NEWS";
my $version = "4.0.0";

my $header = 'BUGFIXES FROM KDE BUGZILLA';
my $url = "https://bugs.kde.org/buglist.cgi?f1=cf_versionfixedin&order=Last%20Changed&o1=equals&query_format=advanced&bug_status=RESOLVED&bug_status=CLOSED&v1=$version&product=digikam&columnlist=short_desc&ctype=csv";

my $ua = LWP::UserAgent->new;
print "Fetching closed bugs for digiKam version ${version} ..\n";
print "URL: $url\n";

my $response = $ua->get($url);
unless($response->is_success) {
    die "Fetching the closed bugs failed: $response->status_line";
}

my $new_content;
my $header_done = 0;
foreach my $line (split(/\n/, read_file($NEWS_FILE))) {
    $new_content .= $line . "\n";
    if($line =~ /^$header.+/) {
        $new_content .= "\n"; # append extra newline
        $header_done = 1;
        last;
    }
}

my $count = 0;
my $fixed_bugs;
my @content = split(/\n/, $response->decoded_content);
foreach my $line (@content) {
    unless($count == 0) {
        my $num = sprintf("%03d", $count);
        my($bugnum, $desc) = $line =~ /(\d+),"(.+)"/;
        $desc =~ s/\"\"/\"/g;
        $fixed_bugs .= "$num ==> $bugnum - $desc\n";
    }
    $count++;
}
$new_content .= $fixed_bugs;
write_file($NEWS_FILE, $new_content);
print "$NEWS_FILE updated.\n"
