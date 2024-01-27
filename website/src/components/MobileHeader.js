import React, { useState } from "react";
import { Tabs, Tab, Navbar, Nav, NavDropdown } from "react-bootstrap";
import { useStaticQuery, graphql } from "gatsby";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import logo from "../images/logo-appview.svg";
import criblLogo from "../images/logo-cribl-new.svg";

import "../scss/_mobileNav.scss";
import "../utils/font-awesome";
export default function MobileHeader() {
  const data = useStaticQuery(graphql`
    query MobileSiteNavQuery {
      allHeaderYaml {
        edges {
          node {
            name
            path
          }
        }
      }
      allCorpSiteNavYaml {
        edges {
          node {
            navigationLeft {
              parent
              child {
                link
                url
              }
            }
          }
        }
      }
    }
  `);
  const [burgerMenu, setBurgerMenu] = useState(false);
  return (
    <nav style={{ position: "relative" }}>
      <FontAwesomeIcon
        icon={["fas", burgerMenu ? "times" : "bars"]}
        style={{
          position: "absolute",
          color: "#000",
          right: burgerMenu ? 16 : 14,
          top: 14,
          fontSize: 36,
          zIndex: 100,
        }}
        onClick={() => {
          setBurgerMenu(burgerMenu ? false : true);
        }}
      />
      <Tabs defaultActiveKey="AppView" id="uncontrolled-tab-example">
        <Tab
          eventKey="AppView"
          title={<img src={logo} alt="Appappview" style={{ width: 100 }} />}
          className={burgerMenu ? "menuActive" : "menuInactive"}
        >
          <Navbar expand="lg">
            <Nav className="mr-auto">
              {data.allHeaderYaml.edges.map((item, i) => {
                return (
                  <Nav.Link key={i} href={item.node.path}>
                    {item.node.name}
                  </Nav.Link>
                );
              })}
            </Nav>
          </Navbar>
        </Tab>
        <Tab
          eventKey="Cribl"
          title={
            <img
              src={criblLogo}
              alt="Appappview"
              style={{ width: 80, color: "#000" }}
            />
          }
          className={burgerMenu ? "menuActive" : "menuInactive"}
        >
          <Navbar expand="lg">
            <Nav className="mr-auto">
              {data.allCorpSiteNavYaml.edges[0].node.navigationLeft.map(
                (item, i) => {
                  return item.child === null ? (
                    <Nav.Link href={item.url} key={i}>
                      {item.parent}
                    </Nav.Link>
                  ) : (
                    <NavDropdown
                      title={item.parent}
                      id="basic-nav-dropdown"
                      key={i}
                    >
                      {item.child.map((childItem, j) => {
                        return (
                          <NavDropdown.Item href={childItem.url} key={j}>
                            {childItem.link}
                          </NavDropdown.Item>
                        );
                      })}
                    </NavDropdown>
                  );
                }
              )}
            </Nav>
          </Navbar>
        </Tab>
      </Tabs>
    </nav>
  );
}
